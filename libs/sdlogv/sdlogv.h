/*
 * theora.h
 * https://gitlab.com/bztsrc/sdlogv
 *
 * Copyright (C) 2023 bzt (bztsrc@gitlab), MIT license
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 * DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * @brief Multithreaded Vorbis / theora decoder header
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_mixer.h>                             /* Mix_Chunk only */
#include <SDL_thread.h>
#include <theora/theoradec.h>
#include <vorbis/codec.h>

/* using circular buffer must be thread safe, because there's only one producer and one consumer,
 * and head and tail are only changed by the producer and consumer respectively but never both */
#define THEORA_QUEUE_SIZE 512

typedef struct {
    uint32_t playms;                                    /* when to play this frame in millisec */
    uint8_t *vbuf;                                      /* data, in SDL_PIXELFORMAT_IYUV planar format */
} theora_frame_t;

typedef struct {
    /* public fields */
    volatile int hasAudio;                              /* has audio stream */
    volatile int hasVideo, w, h;                        /* has video stream, dimensions */
    /* private fields */
    volatile int started, stop, done;                   /* started, request stop to producer and acknowledge flag */
    volatile int ahead, atail, vhead, vtail;            /* circular buffer pointers */
    volatile Mix_Chunk chunk[THEORA_QUEUE_SIZE];        /* audio buffer */
    volatile theora_frame_t frame[THEORA_QUEUE_SIZE];   /* video buffer */
    volatile Uint32 baseticks;                          /* decode start time to get video play time */
    SDL_Thread *th;                                     /* thread identifier */
    FILE *f;                                            /* file stream */
} theora_t;

/**
 * Get some data to parse. Rewrite this function if you don't like file streams
 */
static int theora_getdata(theora_t *ctx, ogg_sync_state *oy)
{
    char *buffer;
    int bytes;

    if(!ctx->f) return 0;
    buffer = ogg_sync_buffer(oy, 4096);
    bytes = fread(buffer, 1, 4096, ctx->f);
    if(bytes <= 0) return 0;
    return !ogg_sync_wrote(oy, bytes);
}

/**
 * Decoder and circular buffer producer
 */
static int theora_producer(void *data)
{
    theora_t *ctx = (theora_t *)data;
    uint8_t *dst, *p;
    int16_t *pcm, ival;
    uint32_t j, w, h;
    float **raw, val;
    int ret, i, doread, uvoff;
    ogg_int64_t      videobuf_granulepos;
    ogg_packet       op;
    ogg_sync_state   oy;
    ogg_page         og;
    ogg_stream_state vo;
    ogg_stream_state to;
    ogg_stream_state test;
    th_info          ti;
    th_comment       tc;
    th_dec_ctx       *td;
    th_setup_info    *ts;
    vorbis_info      vi;
    vorbis_dsp_state vd;
    vorbis_block     vb;
    vorbis_comment   vc;
    th_ycbcr_buffer  ycbcr;

    ogg_sync_init(&oy);
    vorbis_info_init(&vi);
    vorbis_comment_init(&vc);
    th_comment_init(&tc);
    th_info_init(&ti);
    td = NULL; ts = NULL;
    if(ctx->f) fseek(ctx->f, 0, SEEK_SET);

    /* Ogg file open; parse the headers */
    /* Only interested in Vorbis/Theora streams */
    ret = ctx->hasVideo = ctx->hasAudio = ctx->w = ctx->h = doread = 0;
    while (!ctx->stop && !ret) {
        if(!theora_getdata(ctx, &oy)) break;
        while(ogg_sync_pageout(&oy, &og) > 0) {
            if(!ogg_page_bos(&og)) {
                if(ctx->hasVideo) ogg_stream_pagein(&to, &og);
                if(ctx->hasAudio) ogg_stream_pagein(&vo, &og);
                ret = 1;
                break;
            }
            ogg_stream_init(&test, ogg_page_serialno(&og));
            ogg_stream_pagein(&test, &og);
            ogg_stream_packetout(&test, &op);
            if(!ctx->hasVideo && th_decode_headerin(&ti, &tc, &ts, &op) >= 0) {
                memcpy(&to, &test, sizeof(test));
                ctx->hasVideo = 1;
            } else
            if(!ctx->hasAudio && vorbis_synthesis_headerin(&vi, &vc, &op) >= 0) {
                memcpy(&vo, &test, sizeof(test));
                ctx->hasAudio = 1;
            } else
                ogg_stream_clear(&test);
        }
    }
    /* neither audio nor video streams found */
    if(!ctx->hasVideo && !ctx->hasAudio) goto cleanup;

    /* we're expecting more header packets. */
    while(!ctx->stop && ctx->f && !feof(ctx->f) &&
      ((ctx->hasVideo && ctx->hasVideo < 3) || (ctx->hasAudio && ctx->hasAudio < 3))) {
        while(ctx->hasVideo && (ctx->hasVideo < 3)) {
            if(ogg_stream_packetout(&to, &op) != 1 || ctx->stop) break;
            if(!th_decode_headerin(&ti, &tc, &ts, &op)) { ctx->hasVideo = 0; break; }
            ctx->hasVideo++;
        }
        while(ctx->hasAudio && (ctx->hasAudio < 3)) {
            if(ogg_stream_packetout(&vo, &op) != 1 || ctx->stop) break;
            if(vorbis_synthesis_headerin(&vi, &vc, &op)) { ctx->hasAudio = 0; break; }
            ctx->hasAudio++;
        }
        if(ogg_sync_pageout(&oy, &og) > 0) {
            if(ctx->hasVideo) ogg_stream_pagein(&to, &og);
            if(ctx->hasAudio) ogg_stream_pagein(&vo, &og);
        } else
            theora_getdata(ctx, &oy);
    }

    /* and now we have it all.  initialize decoders */
    if(ctx->hasVideo && ti.pixel_fmt == TH_PF_420 && ti.pic_width > 0 && ti.pic_height > 0 &&
      ti.pic_width < 16384 && ti.pic_height < 16384 && (td = th_decode_alloc(&ti, ts))) {
        ret = 0; th_decode_ctl(td, TH_DECCTL_SET_PPLEVEL, &ret, sizeof(ret));   /* turn off post processing */
        ctx->w = ti.pic_width;
        ctx->h = ti.pic_height;
    } else {
        /* tear down the partial theora setup */
        th_info_clear(&ti);
        th_comment_clear(&tc);
        ctx->hasVideo = ctx->w = ctx->h = 0;
    }
    if(ts) { th_setup_free(ts); ts = NULL; }

    if(ctx->hasAudio){
        vorbis_synthesis_init(&vd, &vi);
        if(vi.channels > 2) {
            /* sorry, 5.1 not supported (yet), needs SDL_ConvertAudio, see below */
            vorbis_info_clear(&vi);
            ctx->hasAudio = 0;
        } else
            vorbis_block_init(&vd, &vb);
    } else {
        vorbis_info_clear(&vi);
        vorbis_comment_clear(&vc);
    }
    /********************** that was only the setup so far... now do the real thing **********************/
    ctx->started = 1;

    while(!ctx->stop && (ctx->hasVideo || ctx->hasAudio)) {

        /* read in data */
        if(doread) {
            doread = 0;
            if(theora_getdata(ctx, &oy) > 0)
                while(!ctx->stop && ogg_sync_pageout(&oy, &og) > 0) {
                    if(ctx->hasVideo) ogg_stream_pagein(&to, &og);
                    if(ctx->hasAudio) ogg_stream_pagein(&vo, &og);
                }
            else
                break;
        }

        /*** parse audio ***/
        while(!ctx->stop && ctx->hasAudio && (ctx->ahead + 1) % THEORA_QUEUE_SIZE != ctx->atail) {
            /* if there's pending, decoded audio, grab it */
            if((ret = vorbis_synthesis_pcmout(&vd, &raw)) > 0) {
                vorbis_synthesis_read(&vd, ret);
                ctx->chunk[ctx->ahead].volume = MIX_MAX_VOLUME;
                ctx->chunk[ctx->ahead].allocated = 0;
                ctx->chunk[ctx->ahead].alen = i = sizeof(int16_t) * (vi.channels < 2 ? 2 : vi.channels) * ret;
                ctx->chunk[ctx->ahead].abuf = (Uint8*)realloc(ctx->chunk[ctx->ahead].abuf, i);
                /* deinterlaced float -> interlaced short int pcm */
                pcm =  (int16_t*)ctx->chunk[ctx->ahead].abuf;
                if(pcm) {
                    for(i = 0; i < ret; i++)
                        /* do mono -> stereo the simple and fast way */
                        if(vi.channels == 1) {
                            val = raw[0][i]; ival = (int16_t)(val < -1.0f ? -32768 : (val > 1.0f ? 32767 : val * 32767.0f));
                            *(pcm++) = ival;
                            *(pcm++) = ival;
                        } else
                            for(j = 0; j < (uint32_t)vi.channels; j++) {
                                val = raw[j][i];
                                *(pcm++) = (int16_t)(val < -1.0f ? -32768 : (val > 1.0f ? 32767 : val * 32767.0f));
                            }
                    /* if there are more than stereo channels or frequency doesn't match we must convert */
                    if(vi.rate != 44100 || vi.channels > 2) {
                        /* TODO: set up SDL_ConvertAudio. Should solve 5.1 issue too */
                    }
                    /* add to queue */
                    ctx->ahead = (ctx->ahead + 1) % THEORA_QUEUE_SIZE;
                    ctx->started |= 2;
                }
            } else {
                if(ogg_stream_packetout(&vo, &op) > 0) {
                    if(vorbis_synthesis(&vb, &op) == 0) vorbis_synthesis_blockin(&vd, &vb);
                } else {
                    doread = 1;
                    break;
                }
            }
        }

        /*** parse video ***/
        while(!ctx->stop && ctx->hasVideo && (ctx->vhead + 1) % THEORA_QUEUE_SIZE != ctx->vtail) {
            /* theora is one in, one out... */
            if(ogg_stream_packetout(&to, &op) > 0) {
                if(op.granulepos >= 0)
                    th_decode_ctl(td, TH_DECCTL_SET_GRANPOS, &op.granulepos, sizeof(op.granulepos));
                videobuf_granulepos = 0;
                if(th_decode_packetin(td, &op, &videobuf_granulepos) == 0) {
                    if(th_decode_ycbcr_out(td, ycbcr) == 0) {
                        ctx->frame[ctx->vhead].playms = (uint32_t)(th_granule_time(td, videobuf_granulepos) * 1000.0);
                        if(!ctx->frame[ctx->vhead].vbuf)
                            ctx->frame[ctx->vhead].vbuf = (uint8_t*)malloc(ti.pic_width * ti.pic_height * 2);
                        dst = ctx->frame[ctx->vhead].vbuf;
                        if(dst) {
                            /* frame420 to YUVPlanar */
                            p = ycbcr[0].data + (ti.pic_x & ~1) + ycbcr[0].stride * (ti.pic_y & ~1);
                            uvoff = (ti.pic_x / 2) + (ycbcr[1].stride) * (ti.pic_y / 2);
                            w = ti.pic_width / 2; h = ti.pic_height / 2;
                            for(j = 0; j < ti.pic_height; j++, dst += ti.pic_width, p += ycbcr[0].stride)
                                memcpy(dst, p, ti.pic_width);
                            for(j = 0, p = ycbcr[1].data + uvoff; j < h; j++, dst += w, p += ycbcr[1].stride) memcpy(dst, p, w);
                            for(j = 0, p = ycbcr[2].data + uvoff; j < h; j++, dst += w, p += ycbcr[2].stride) memcpy(dst, p, w);
                            /* add to queue */
                            ctx->vhead = (ctx->vhead + 1) % THEORA_QUEUE_SIZE;
                            ctx->started |= 4;
                        }
                    }
                }
            } else {
                doread = 1;
                break;
            }
        }
    }

    /**************************************** clean up *****************************************/
cleanup:
    if(ts) { th_setup_free(ts); ts = NULL; }

    if(ctx->hasAudio) {
        ogg_stream_clear(&vo);
        vorbis_block_clear(&vb);
        vorbis_dsp_clear(&vd);
    }
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    if(ctx->hasVideo) ogg_stream_clear(&to);
    if(td) { th_decode_free(td); td = NULL; }
    th_comment_clear(&tc);
    th_info_clear(&ti);
    ogg_sync_clear(&oy);

    ctx->started = ctx->done = 1;
    return 0;
}

/**
 * Start decoder
 */
void theora_start(theora_t *ctx, FILE *f)
{
    if(!ctx || !f) return;
    ctx->f = f;
    ctx->stop = ctx->done = ctx->started = ctx->hasVideo = ctx->hasAudio = 0;
    ctx->th = SDL_CreateThread(theora_producer, "producer", ctx);
    /* wait until the producer thread detects the streams and sets hasAudio and hasVideo flags */
    while(!ctx->started) SDL_Delay(10);
    /* wait until there's some data in the circular buffers to consume */
    while(!ctx->done && ctx->started != (1 | (ctx->hasAudio ? 2 : 0) | (ctx->hasVideo ? 4 : 0))) SDL_Delay(10);
    ctx->baseticks = SDL_GetTicks();
}

/**
 * Stop decoder
 */
void theora_stop(theora_t *ctx)
{
    int i;

    if(!ctx) return;
    ctx->stop = 1;
    if(ctx->th) { SDL_WaitThread(ctx->th, &i); ctx->th = NULL; }
    /* failsafe */
    while(!ctx->done) SDL_Delay(10);
    /* free internal buffers */
    for(i = 0; i < THEORA_QUEUE_SIZE; i++) {
        if(ctx->chunk[i].abuf) { free(ctx->chunk[i].abuf); ctx->chunk[i].abuf = NULL; }
        if(ctx->frame[i].vbuf) { free(ctx->frame[i].vbuf); ctx->frame[i].vbuf = NULL; }
    }
    ctx->stop = ctx->done = ctx->started = ctx->ahead = ctx->atail = ctx->vhead = ctx->vtail = ctx->hasAudio = ctx->hasVideo = 0;
}

/**
 * Returns false if playing is finished (that is, decoding is done and everything is consumed from the buffers)
 */
int theora_playing(theora_t *ctx)
{
    return ctx && !ctx->stop && (!ctx->done || ctx->ahead != ctx->atail || ctx->vhead != ctx->vtail);
}

/**
 * Consume data from audio buffer
 */
Mix_Chunk *theora_audio(theora_t *ctx)
{
    volatile Mix_Chunk *ret;

    if(!ctx || !ctx->hasAudio || ctx->stop) return NULL;
    /* this is bad. we may have consumed the audio faster than the producer could produce. Wait for a bit. */
    while(!ctx->done && ctx->atail == ctx->ahead);
    /* if the queue is still empty, that means end of audio */
    if(ctx->atail == ctx->ahead) return NULL;
    ret = &ctx->chunk[ctx->atail];
    ctx->atail = (ctx->atail + 1) % THEORA_QUEUE_SIZE;
    return (Mix_Chunk*)ret;
}

/**
 * Consume data from video buffer and update a texture with it
 */
void theora_video(theora_t *ctx, SDL_Texture *texture)
{
    int i, pitch = 0;
    Uint8 *y, *u, *v, *dst = NULL;
    Uint32 now;

    /* here it is not a problem if the queue is temporarily empty, since we're not running inside SDL mixer callbacks */
    if(!ctx || !texture || !ctx->hasVideo || ctx->stop || ctx->vtail == ctx->vhead) return;
    now = SDL_GetTicks() - ctx->baseticks;
    if(ctx->frame[ctx->vtail].playms > now) return;
    /* handle frame drop, when the next frame is in the past too */
    while((ctx->vtail + 1) % THEORA_QUEUE_SIZE != ctx->vhead && ctx->frame[(ctx->vtail + 1) % THEORA_QUEUE_SIZE].vbuf &&
      ctx->frame[(ctx->vtail + 1) % THEORA_QUEUE_SIZE].playms < now) ctx->vtail = (ctx->vtail + 1) % THEORA_QUEUE_SIZE;
    if(ctx->vtail == ctx->vhead || !ctx->frame[ctx->vtail].vbuf) return;
    /* update the texture with the current frame */
    i = ctx->vtail; ctx->vtail = (ctx->vtail + 1) % THEORA_QUEUE_SIZE;
    SDL_LockTexture(texture, NULL, (void**)&dst, &pitch);
    /* failsafe if SDL failed to lock the texture */
    if(dst && pitch > 0) {
        y = ctx->frame[i].vbuf;
        u = y + (ctx->w * ctx->h);
        v = u + ((ctx->w / 2) * (ctx->h / 2));
        for (i = 0; i < ctx->h; i++, y += ctx->w, dst += pitch) memcpy(dst, y, ctx->w);
        for (i = 0; i < ctx->h / 2; i++, u += ctx->w / 2, dst += pitch / 2) memcpy(dst, u, ctx->w / 2);
        for (i = 0; i < ctx->h / 2; i++, v += ctx->w / 2, dst += pitch / 2) memcpy(dst, v, ctx->w / 2);
    }
    SDL_UnlockTexture(texture);
}

/**
 * Get the duration of an ogg file in millisec
 */
# define TH_VERSION_CHECK(_info,_maj,_min,_sub) \
 ((_info)->version_major>(_maj)||((_info)->version_major==(_maj)&& \
 (((_info)->version_minor>(_min)||((_info)->version_minor==(_min)&& \
 (_info)->version_subminor>=(_sub))))))
uint64_t theora_getduration(FILE *f)
{
    uint64_t size, dur = 0;
    uint8_t *buff;
    char *buffer;
    int hv, ha, s, sv = 0, sa = 0;
    ogg_int64_t      granulepos, iframe, pframe;
    ogg_packet       op;
    ogg_sync_state   oy;
    ogg_page         og;
    ogg_stream_state vo;
    ogg_stream_state to;
    ogg_stream_state test;
    th_info          ti;
    th_comment       tc;
    th_setup_info    *ts;
    vorbis_info      vi;
    vorbis_dsp_state vd;
    vorbis_comment   vc;

    if(!f) return 0;
    fseek(f, 0, SEEK_END);
    size = (uint64_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    ogg_sync_init(&oy);
    vorbis_info_init(&vi);
    vorbis_comment_init(&vc);
    th_comment_init(&tc);
    th_info_init(&ti);
    ts = NULL;

    /* Ogg file open; parse the headers */
    /* Only interested in Vorbis/Theora streams */
    s = hv = ha = 0;
    while (!s) {
        buffer = ogg_sync_buffer(&oy, 4096);
        s = fread(buffer, 1, 4096, f);
        if(s <= 0 || ogg_sync_wrote(&oy, s)) break;
        s = 0;
        while(ogg_sync_pageout(&oy, &og) > 0) {
            if(!ogg_page_bos(&og)) {
                if(hv) ogg_stream_pagein(&to, &og);
                if(ha) ogg_stream_pagein(&vo, &og);
                s = 1;
                break;
            }
            ogg_stream_init(&test, ogg_page_serialno(&og));
            ogg_stream_pagein(&test, &og);
            ogg_stream_packetout(&test, &op);
            if(!hv && th_decode_headerin(&ti, &tc, &ts, &op) >= 0) {
                memcpy(&to, &test, sizeof(test));
                sv = ogg_page_serialno(&og);
                hv = 1;
            } else
            if(!ha && vorbis_synthesis_headerin(&vi, &vc, &op) >= 0) {
                memcpy(&vo, &test, sizeof(test));
                sa = ogg_page_serialno(&og);
                ha = 1;
            } else
                ogg_stream_clear(&test);
        }
    }
    if(ha || hv) {
        /* we're expecting more header packets. */
        while(!feof(f) && ((hv && hv < 3) || (ha && ha < 3))) {
            while(hv && (hv < 3)) {
                if(ogg_stream_packetout(&to, &op) != 1) break;
                if(!th_decode_headerin(&ti, &tc, &ts, &op)) { hv = 0; break; }
                hv++;
            }
            while(ha && (ha < 3)) {
                if(ogg_stream_packetout(&vo, &op) != 1) break;
                if(vorbis_synthesis_headerin(&vi, &vc, &op)) { ha = 0; break; }
                ha++;
            }
            if(ogg_sync_pageout(&oy, &og) > 0) {
                if(hv) ogg_stream_pagein(&to, &og);
                if(ha) ogg_stream_pagein(&vo, &og);
            } else {
                buffer = ogg_sync_buffer(&oy, 4096);
                s = fread(buffer, 1, 4096, f);
                if(s <= 0 || ogg_sync_wrote(&oy, s)) break;
            }
        }
        /* and now we have it all.  initialize ti and vi structures */
        if(!(hv && ti.pixel_fmt == TH_PF_420 && ti.pic_width > 0 && ti.pic_height > 0 &&
          ti.pic_width < 16384 && ti.pic_height < 16384)) {
            /* tear down the partial theora setup */
            th_info_clear(&ti);
            th_comment_clear(&tc);
            hv = 0;
        }
        if(ts) th_setup_free(ts);

        if(ha){
            vorbis_synthesis_init(&vd, &vi);
        } else {
            vorbis_info_clear(&vi);
            vorbis_comment_clear(&vc);
        }
        /* read in the last 128K of the file. Forget tht horrible ogg_sync API from now on */
        if(size > 128 * 1024) {
            size = 128 * 1024;
            fseek(f, -128 * 1024, SEEK_END);
        } else {
            fseek(f, 0, SEEK_SET);
        }
        buff = (uint8_t*)malloc(size);
        if(buff) {
            fread(buff, size, 1, f);
            /* locate the last ogg packet */
            s = (hv ? sv : sa);
            for(og.header = buff + size - 19; og.header > buff && (memcmp(og.header, "OggS", 4) || ogg_page_serialno(&og) != s);
                og.header--);
            if(og.header > buff) {
                granulepos = ogg_page_granulepos(&og);
                /* if we have found the last packet of the audio stream */
                if(s == sa) {
                    dur = (uint64_t)((granulepos * 1000 + vi.rate - 1) / vi.rate);
                } else
                /* if we have found the last packet of the video stream */
                if(s == sv) {
                    /* this is from th_granule_time() */
                    iframe = granulepos >> ti.keyframe_granule_shift;
                    pframe = granulepos - (iframe << ti.keyframe_granule_shift);
                    granulepos = iframe + pframe - TH_VERSION_CHECK(&ti,3,2,1);
                    if(ti.fps_numerator < 1) ti.fps_numerator = 1;
                    dur = (uint64_t)(((granulepos + 1) * 1000 * ti.fps_denominator + ti.fps_numerator - 1) / ti.fps_numerator);
                }
            }
            free(buff);
        }
    }
    fseek(f, 0, SEEK_SET);
    if(ha) {
        ogg_stream_clear(&vo);
        vorbis_dsp_clear(&vd);
    }
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    if(hv) ogg_stream_clear(&to);
    th_comment_clear(&tc);
    th_info_clear(&ti);
    ogg_sync_clear(&oy);

    return dur;
}
