#include "graphics.h"

#include "utils/util_bits.h"
#include "utils/util_time.h"


void update_texture(SDL_Texture* texture, uint32_t* pixel, uint32_t height)
{
    void* dest_pixel;
    int pitch;

    SDL_LockTexture(texture, NULL, &dest_pixel, &pitch);
    memcpy(dest_pixel, pixel, pitch * height);
    SDL_UnlockTexture(texture);
}

void update_pixel(System_Context* context, uint32_t* pixel)
{
    uint32_t size = context->display.size;
    for (uint32_t i = 0; i < size; i++)
    {
        if (test_bit(context->display.buffer, i))
            pixel[i] = 0xFFFFFFFF;
        else
            pixel[i] = 0x00000000;
    }
}

/* This is the only public function. */
uint32_t update_graphics_wrapper(uint32_t interval, void* param)
{
    System_Context* context = ((struct graphics_thread_params*) param)->context;
    SDL_Texture* texture = ((struct graphics_thread_params*) param)->texture;
    SDL_Renderer* renderer = ((struct graphics_thread_params*) param)->renderer;
    uint32_t* pixel = ((struct graphics_thread_params*) param)->pixel;

    pthread_mutex_lock(&context->mutex);
    if (context->display.updated)
    {
        SDL_RenderClear(renderer);
        update_pixel(context, pixel);
        update_texture(texture, pixel, context->display.height);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        context->display.updated = false;
    }

    pthread_cond_broadcast(&context->vsyncCond);
    update_timer(context);

    pthread_mutex_unlock(&context->mutex);

    return MS_PER_SECOND / DISPLAY_FREQUENCY;
}