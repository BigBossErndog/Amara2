/*
 * main.c
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
 * @brief A simple videoplayer that uses my theora.h wrapper
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <sdlogv.h>

/**
 * The decoder context. Must be global because SDL mixer callbacks know no better
 */
theora_t ctx = { 0 };

/**
 * The SDL mixer's callback, which, thanks to the wrapper is brainfuck simple.
 */
static void callback(int channel)
{
    Mix_Chunk *audio = theora_audio(&ctx);
    /* could we get some audio data? If so, play it! */
    if(audio)
        Mix_PlayChannel(channel, audio, 0);
}

/* just sets a flag */
int got_sigint = 0;
static void sigint_handler (int signal) { got_sigint = signal; }

/**
 * Example on how to use the wrapper
 */
int main(int argc, char **argv)
{
    FILE *f;
    SDL_Renderer *renderer = NULL;  /* the usual SDL stuff. */
    SDL_Texture *texture = NULL;
    SDL_Window *window = NULL;
    SDL_Event event;
    SDL_DisplayMode dm;
    SDL_Rect rect;                  /* the rectangle where we want to display the video */

    char* path = "assets/pitv_trailer.ogv";

    /* open the video file */
    f = fopen(path, "rb");
    if(!f) {
        fprintf(stderr, "unable to open file\n");
        exit(1);
    }

    /************************************************************************/
    /***                  get the duration in millisec                    ***/
    /************************************************************************/
    printf("Duration: %lu msec\n", theora_getduration(f));

    /* nothing to see here, please move on, just initialize SDL2 and SDL mixer */
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_EVENTS);
    SDL_GetDesktopDisplayMode(0, &dm);
    Mix_Init(0);
    Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096);

    /************************************************************************/
    /***                       start the decoder                          ***/
    /************************************************************************/
    /* yeah, really this simple. */
    theora_start(&ctx, f);

    /* Is there a video stream in the file? If so, open a window and create a texture where we will paint the video */
    if(ctx.hasVideo) {
        SDL_GetDesktopDisplayMode(0, &dm);
        window = SDL_CreateWindow("sdlogv", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w, dm.h, SDL_WINDOW_FULLSCREEN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        /* the decoder was kind enough to tell us the video dimensions in advance */
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, ctx.w, ctx.h);
        /* fit it to our screen centered keeping aspect ratio */
        rect.w = dm.w; rect.h = ctx.h * dm.w / ctx.w;
        if(rect.h > dm.h) { rect.h = dm.h; rect.w = ctx.w * dm.h / ctx.h; }
        rect.x = (dm.w - rect.w) / 2; rect.y = (dm.h - rect.h) / 2;
    } else {
        /* in lack of a window, SDL can't handle keypress events. */
        signal(SIGINT, sigint_handler);
    }

    /************************************************************************/
    /***                          audio player                            ***/
    /************************************************************************/

    /* this is going to be extremely simple, because we're using SDL mixer to play it in the background */
    if(ctx.hasAudio) {
        Mix_ChannelFinished(callback);
        callback(0);
    }

    /************************************************************************/
    /***                          video player                            ***/
    /************************************************************************/

    printf("Playing started...\n");
    /* in this simple example, we quit when we are finished playing, but you don't have to */
    while (theora_playing(&ctx) && !got_sigint) {

        /* is there a frame to play? If so, refresh the texture */
        theora_video(&ctx, texture);

        /* handle some events... */
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT || event.type == SDL_KEYUP || event.type == SDL_MOUSEBUTTONDOWN) break;

        /* render as usual. The video is just another texture to add. You could resize and position it if you want */
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_RenderPresent(renderer);
    }
    printf("Finished playing\n");

    /************************************************************************/
    /***                        stop the decoder                          ***/
    /************************************************************************/

    /* the theora thread has absolutely no clue about the SDL mixer usings its buffers, so we MUST stop the mixer first. */
    if(Mix_Playing(-1)) {
        Mix_ChannelFinished(NULL);
        Mix_HaltChannel(-1);
    }

    /* ok, now that nobody is relying on the audio buffer, we can stop the decoder and destroy the context */
    theora_stop(&ctx);

    /* as usual, free SDL resources */
    printf("Tear down\n");
    fclose(f);
    if(texture) SDL_DestroyTexture(texture);
    if(renderer) SDL_DestroyRenderer(renderer);
    if(window) SDL_DestroyWindow(window);
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
    return 0;
}
