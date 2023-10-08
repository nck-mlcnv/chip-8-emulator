#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <SDL2/sdl.h>
#include <time.h>
#include <pthread.h>

#include <windows.h>

#include "emulator.h"
#include "input.h"
#include "graphics.h"
#include "utils/util_time.h"

#define TIMER_FREQUENCY 60
#define TICK_FREQUENCY 100


struct tick_thread_params {
    System_Context* context;
    bool* running;
    bool* step_mode;
};

void* tick_loop(void* param)
{
    System_Context* context = ((struct tick_thread_params*) param)->context;
    bool* running = ((struct tick_thread_params*) param)->running;
    bool* step_mode = ((struct tick_thread_params*) param)->step_mode;

    struct timespec tick_last, tick_now, tick_delta;
    clock_gettime(CLOCK_MONOTONIC, &tick_last);

    while (*running)
    {
        clock_gettime(CLOCK_MONOTONIC, &tick_now);
        sub_timespec(&tick_last, &tick_now, &tick_delta);
        if (tick_delta.tv_sec > 1 || tick_delta.tv_nsec > NS_PER_SECOND / TICK_FREQUENCY)
        {
            if(!(*step_mode)) tick(context);
            clock_gettime(CLOCK_MONOTONIC, &tick_last);
        }

        Sleep(1);
    }
}

int main(int argc, char* args[])
{
    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("Error during initialization of SDL.");
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Hallo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 400, 0);
    if (window == NULL)
    {
        printf("Error initializing the window.");
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL)
    {
        printf("Error during initialization of the renderer.");
        return -1;
    }

    /* Setup emulator */
    const char* file_path = "..\\res\\5-quirks.ch8";
    System_Context context;
    load_rom(&context, file_path);
    initialise(&context);
    uint32_t pixel[context.display.size];

    /* More SDL stuff */
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, context.display.width, context.display.height);
    SDL_SetTextureColorMod(texture, 200, 200, 200);
    SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);


    /* Main loop variables */
    SDL_Event event;
    bool running = true;
    bool step_mode = false;

    /* Start SDL Timer. */
    // SDL_TimerID timerID1 = SDL_AddTimer(MS_PER_SECOND / TIMER_FREQUENCY, update_timer_wrapper, &context);
    struct graphics_thread_params graph_params = {&context, texture, renderer, pixel };
    SDL_TimerID timerID2 = SDL_AddTimer(MS_PER_SECOND / TIMER_FREQUENCY, update_graphics_wrapper, &graph_params);

    /* Start main emulator tick loop thread. */
    pthread_t emulator_tick_thread;
    struct tick_thread_params tick_thread_params = { &context, &running, &step_mode };
    pthread_create(&emulator_tick_thread, NULL, tick_loop, &tick_thread_params);

    /* SDL Window loop with event loop. */
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            {
                running = false;
            }
            else if (event.type == SDL_KEYUP && event.key.keysym.scancode == SDL_SCANCODE_SPACE)
            {
                step_mode ^= 1;
            }
            else if (event.type == SDL_KEYUP && event.key.keysym.scancode == SDL_SCANCODE_N && step_mode)
            {
                tick(&context);
            }
            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                handle_input(&context, &event);
            }
        }
        if (pthread_kill(emulator_tick_thread, 0) == ESRCH) break; // check if tick thread is dead
        Sleep(1);
    }

    // SDL_RemoveTimer(timerID1);
    SDL_RemoveTimer(timerID2);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    pthread_join(emulator_tick_thread, NULL);

    destruct(&context);

    return 0;
}
