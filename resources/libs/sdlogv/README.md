SDL Theora Video (OGV) Decoder Thread
=====================================

What's this?
------------

Let's assume you have an SDL application in which you wish to play a video file in an area pointed by an SDL_rect. I don't
understand why that has to be so difficult. It should be simple. I like simple. K.I.S.S.!

There's an SDL example in libtheora's repo, [player_example.c](https://github.com/xiph/theora/blob/master/examples/player_example.c),
but it's using obsoleted surfaces and not the hardware accelerated SDL renderer. Besides, that code isn't simple and easy to
follow, rather something we can safely call disgusting. It could be nominated for ICCC for Christ's sake.

Others thought this too, so there's [TheoraPlay](https://github.com/icculus/theoraplay) which supposed to make decoding easier.
Well, it does not help a lot with that. It is VERY overengineered and known to easily have memory issues in unexperienced hands.
You must be extremely careful when you wish to integrate it with SDL mixer. Not trivial at least to say. And that's only if
you use it on Linux or Windows, because other platforms might not work at all (has its own threading wrapper instead of relying
on SDL_Thread).

So, long story short, here we go again, me wrapping up things with a friendly API. This stb-style, single header file
[theora.h](https://gitlab.com/bztsrc/sdlogv/blob/main/theora.h) "library" is extremely small, 15 kilobytes (about 300 SLoC) only
and painless to integrate and really easy to use. It is lock-free, and uses an SDL_Thread to decode the stream in the background.

There's an example ogv player application, [main.c](https://gitlab.com/bztsrc/sdlogv/blob/main/main.c) that demonstrates how to
use this header file. Unlike the examples for the above wrappers, this player example is REALLY small and easy to understand
(ca. 100 SLoC).

Dependencies
------------

Minimal, just the most necessary things you would need for playing video files anyway:

- ogg: the container format
- vorbis: audio codec
- theora, theoradec: the video codec (decoder only part)
- SDL2_mixer: is optional, its header is only needed for the Mix_Chunk struct definition, nothing else
- an SDL_rect and an SDL_Texture: is where you want to display the video file

API
---

### Starting and Stopping the Decoder Thread

```c
void theora_start(theora_t *ctx, FILE *f);
```
You pass the context and a file stream which you have opened with the video file. This function is totally bullet proof,
and it waits to return until there's at least some decoded data in its internal buffers (unless there are no ogg streams
in the file). There's no need to do magic like with TheoraPlay before you could enter you main loop.

NOTE: you can use `fmemopen()` to pass a memory buffer to it. Or you could rewrite the buffer feeder to use any source you want.

This context has two flags to tell you if a given ogg stream has audio (vorbis) or video (theora) streams in it:

- `ctx.hasAudio` is set when there's audio
- `ctx.hasVideo` is set when there's video

And to quit the thread, use:
```c
void theora_stop(theora_t *ctx);
```
Yeah that's all. It will free all the memory it has allocated, bullet proof, simple just the way I like it.

To know if there's something to play at all, use
```c
int theora_playing(theora_t *ctx);
```

I got fed up how messed up the ogg/vorbis/theora API is, so I've added another convenience function to return the duration
of an ogg stream in milliseconds (it is ridicolous how complicated this simple function is... and there are NO examples on
the net):
```c
uint64_t theora_getduration(FILE *f);
```
Seriously, what were those Xiph guys thinking? Would it hurt to add a stream's max granule into the ogg headers?

### Playing Audio

```c
Mix_Chunk *theora_audio(theora_t *ctx);
```
This function is called to get decoded data in a format that SDL mixer understands. You should call this repeatedly until
it returns NULL. The simplest way is doing that inside an SDL mixer callback.

```c
/* SDL mixer callback */
void callback(int channel)
{
    Mix_Chunk *audio = theora_audio(&ctx);
    if(audio)
        Mix_PlayChannel(channel, audio, 0);
}

/* tell SDL mixer our callback function */
Mix_ChannelFinished(callback);
```
Now that this has been set up, to actually start the audio playback, all you need to do is calling the callback manually once
(specifying a channel on which to play the theora's audio):
```c
callback(0);
```
After that you can use the usual SDL mixer functions to pause, resume etc. the channel. To entirely stop the playback, just
call `Mix_ExpireChannel()` as you would with any other channels.

IMPORTANT: the theora decoder thread has no clue on who and how is playing its decoded data, so it is VERY important that you
stop the playback before you call `theora_stop()`, otherwise SDL mixer will crash.

### Playing Video

```c
void theora_video(theora_t *ctx, SDL_Texture *texture);
```
This function is called to update the texture if there's a new frame to be displayed. You should call this repeatedly in your
main loop, but first of all, create a texture and render it as you would with any other textures. The video's width and
height is stored in `ctx.w` and `ctx.h` fields of the context respectively.
```c
texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, ctx.w, ctx.h);

theora_video(&ctx, texture);
SDL_RenderCopy(renderer, texture, NULL, &rect);
```

This is all folks!

License
-------

Licensed under the permissive terms of MIT license. Attribution not required, but highly appreciated.

Authors
-------

- bzt (I've used the player_example.c in libtheora as reference, but otherwise I wrote everything).
