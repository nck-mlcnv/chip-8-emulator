#ifndef EMULATOR_GRAPHICS_H
#define EMULATOR_GRAPHICS_H

#include <SDL2/sdl.h>

#include "emulator.h"


struct graphics_thread_params {
    System_Context* context;
    SDL_Texture* texture;
    SDL_Renderer* renderer;
    uint32_t* pixel;
};


uint32_t update_graphics_wrapper(uint32_t interval, void* param);


#endif //EMULATOR_GRAPHICS_H
