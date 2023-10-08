#ifndef EMULATOR_EMULATOR_H
#define EMULATOR_EMULATOR_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>

#include "utils/util_types.h"

#define VSYNC (1)
#define DISPLAY_FREQUENCY 20
#define USED_VERSION VERSION_SUPERCHIP


typedef enum Version {
    VERSION_CHIP8, VERSION_CHIP8_MODERN, VERSION_SUPERCHIP // superchip-1.1 actually
} Version;

typedef struct Keypad {
    uint16_t keys;
    uint8_t last_key;
} Keypad;

typedef struct Display {
    byte* buffer;
    uint32_t width;
    uint32_t height;
    uint32_t size;
    bool updated;   // only redraw to window, if display actually updated
} Display;

typedef struct System_Context {
    byte memory[4096];
    word stack[16];

    word PC;        // program counter
    byte SP;        // stack pointer
    byte reg_V[16]; // general purpose register
    word reg_I;     // index register
    byte reg_DT;    // delay timer
    byte reg_ST;    // sound timer

    bool hires;     // hires mode for superchip

    Display display;
    Keypad keypad;

    word font_adresses[16];
    word big_font_adresses[10];

    Version version;    // which version of chip8 is supposed to be emulated

    bool vsync;

    // thread management
    pthread_cond_t vsyncCond;
    pthread_cond_t haltCond;
    pthread_mutex_t mutex;
} System_Context;


void clear_display(System_Context* context);
void draw_sprite(System_Context* context, uint16_t instruction);
void update_timer(System_Context* context);
void tick(System_Context* context);
void initialise(System_Context* context);
void load_program(System_Context* context, byte* program, uint32_t program_size);
void load_rom(System_Context* context, const char* path);
void destruct(System_Context* context);


#endif //EMULATOR_EMULATOR_H
