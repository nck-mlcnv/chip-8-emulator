#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>

#include "emulator.h"

#include "utils/util_bits.h"

#define PROGRAM_START_INDEX 0x200


void execute_instruction(System_Context* context, word instruction);


void load_rom(System_Context* context, const char* path)
{
    FILE* file_ptr = fopen(path, "rb");
    if (file_ptr == NULL)
    {
        printf("Error while opening file.");
        exit(-1);
    }

    struct stat st;
    fstat(fileno(file_ptr), &st);
    int file_size = st.st_size;

    uint8_t* program = malloc(file_size);
    fread(program, file_size, 1, file_ptr);
    fclose(file_ptr);

    load_program(context, program, file_size);
    free(program);
}

void clear_display(System_Context* context)
{
    if (context->vsync)
    {
        pthread_mutex_lock(&context->mutex);
        pthread_cond_wait(&context->vsyncCond, &context->mutex);
    }

    for (int i = 0; i < 8 * 32; i++)
    {
        context->display.buffer[i] = 0;
    }
    context->display.updated = true;
    if (context->vsync) pthread_mutex_unlock(&context->mutex);
}

void draw_sprite(System_Context* context, uint16_t instruction)
{
    if (context->version == VERSION_SUPERCHIP)
    {
        uint8_t n = instruction & 0x000F;
        if (n == 0) n = 16;
        uint8_t coord_x, coord_y, limit_x, limit_y;
        if (context->hires)
        {
            coord_x = context->reg_V[(instruction & 0x0F00) >> 8] % 128;    // wrap starting pos
            coord_y = context->reg_V[(instruction & 0x00F0) >> 4] % 64;
            if (n == 16)
                limit_x = coord_x + 16 < 128 ? coord_x + 16 : 128;
            else
                limit_x = coord_x + 8 < 128 ? coord_x + 8 : 128;
            limit_y = coord_y + n < 64 ? coord_y + n : 64;
        }
        else
        {
            coord_x = (context->reg_V[(instruction & 0x0F00) >> 8] * 2) % 128;  // scale position in lores
            coord_y = (context->reg_V[(instruction & 0x00F0) >> 4] * 2) % 64;
            limit_x = coord_x + 16 < 128 ? coord_x + 16 : 128;
            limit_y = coord_y + (n * 2) < 64 ? coord_y + (n * 2) : 64;
        }
        int reg_vf = 0;
        int count_y = 0;
        byte *sprite = malloc(n);

        memcpy(sprite, &context->memory[context->reg_I], context->hires && n == 16 ? n * 2: n);
        for (int y = coord_y; y < limit_y; y++) {
            int count_x = 0;
            for (int x = coord_x; x < limit_x; x++) {
                uint32_t display_bit = (y * 64) + x;
                uint32_t sprite_bit = context->hires ? (count_y * 8) + count_x : ((count_y / 2) * 8) + (count_x / 2);
                if (!reg_vf)
                    reg_vf = test_bit(context->display.buffer, display_bit) && test_bit(sprite, sprite_bit);
                xor_bit(context->display.buffer, display_bit, test_bit(sprite, sprite_bit));
                count_x++;
            }
            count_y++;
        }
        free(sprite);
        context->reg_V[0xF] = reg_vf;
        context->display.updated = true;
    }
    else
    {
        if (context->vsync)
        {
            pthread_mutex_lock(&context->mutex);
            pthread_cond_wait(&context->vsyncCond, &context->mutex);
        }

        uint8_t coord_x = context->reg_V[(instruction & 0x0F00) >> 8] % 64; // wrap starting pos
        uint8_t coord_y = context->reg_V[(instruction & 0x00F0) >> 4] % 32;
        uint8_t n = instruction & 0x000F;
        byte *sprite = malloc(n);
        memcpy(sprite, &context->memory[context->reg_I], n);

        int lim_x = coord_x + 8 < 64 ? coord_x + 8 : 64;
        int lim_y = coord_y + n < 32 ? coord_y + n : 32;

        int reg_vf = 0;

        int count_y = 0;
        for (int y = coord_y; y < lim_y; y++) {
            int count_x = 0;
            for (int x = coord_x; x < lim_x; x++) {
                uint32_t bit = (y * 64) + x;
                if (!reg_vf)
                    reg_vf = test_bit(context->display.buffer, bit) && test_bit(sprite, (count_y * 8) + count_x);
                xor_bit(context->display.buffer, bit, test_bit(sprite, (count_y * 8) + count_x));
                count_x++;
            }
            count_y++;
        }
        free(sprite);
        context->reg_V[0xF] = reg_vf;
        context->display.updated = true;
        if (context->vsync) pthread_mutex_unlock(&context->mutex);
    }
}

void update_timer(System_Context* context)
{
    if (context->reg_ST != 0)
        context->reg_ST--;
    if (context->reg_DT != 0)
        context->reg_DT--;
}

void initialise(System_Context* context)
{
    context->version = USED_VERSION;
    context->PC = PROGRAM_START_INDEX;
    context->display.updated = true;
    srand(time(NULL));
    byte fonts[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    memcpy(context->memory, fonts, 5 * 16);
    for (int i = 0; i < 16; i++)
    {
        context->font_adresses[i] = i * 5;
        context->reg_V[i] = 0;
        context->stack[i] = 0;
    }
    context->reg_I = 0;
    context->SP = 0;
    context->reg_DT = 0;
    context->reg_ST = 0;
    context->keypad.keys = 0b0000000000000000;
    context->keypad.last_key = 0;
    context->hires = false;

    if (context->version == VERSION_CHIP8)
        context->vsync = true && VSYNC;
    else
        context->vsync = false;

    if (context->version == VERSION_CHIP8 | context->version == VERSION_CHIP8_MODERN)
    {
        context->display.buffer = calloc(8 * 32, 1);
        context->display.width = 64;
        context->display.height = 32;
        context->display.size = 64 * 32;
    }
    if (context->version == VERSION_SUPERCHIP)
    {
        context->display.buffer = calloc(16 * 64, 1);
        context->display.width = 128;
        context->display.height = 64;
        context->display.size = 128 * 64;
    }



    context->vsyncCond = PTHREAD_COND_INITIALIZER;
    context->haltCond = PTHREAD_COND_INITIALIZER;
    context->mutex = PTHREAD_MUTEX_INITIALIZER;
}

void destruct(System_Context* context)
{
    free(context->display.buffer);
}

void load_program(System_Context* context, byte* program, uint32_t program_size)
{
    memcpy(&context->memory[PROGRAM_START_INDEX], program, program_size);
}

void tick(System_Context* context)
{
    uint16_t instruction = context->memory[context->PC++];
    instruction = instruction << 8;
    instruction = instruction | context->memory[context->PC++];
    execute_instruction(context, instruction);
}

void execute_instruction(System_Context* context, word instruction)
{
    printf("Executing Instruction %04x, v0: %i\n", instruction, context->reg_V[0]);
    uint16_t msb;
    bool temp;
    switch (instruction)
    {
        case 0x00E0:
            clear_display(context);
            break;
        case 0x00EE:
            context->SP -= 1;
            context->PC = context->stack[context->SP];
            break;
        case 0x00FD:
            if (context->version == VERSION_SUPERCHIP)
                pthread_kill(pthread_self(), 9); // sigkill
            else
                printf("Unsupported instruction 0x%04x for current used version.\n", instruction);
            break;
        case 0x00FE:
            if (context->version == VERSION_SUPERCHIP)
            {
                context->hires = false;
            }
            else
                printf("Unsupported instruction 0x%04x for current used version.\n", instruction);
            break;
        case 0x00FF:
            if (context->version == VERSION_SUPERCHIP)
            {
                context->hires = true;
            }
            else
                printf("Unsupported instruction 0x%04x for current used version.\n", instruction);
            break;
        default:
            msb = instruction & 0b1111000000000000;
            switch (msb)
            {
                case (0x1000):
                    context->PC = instruction & 0x0FFF;
                    break;
                case (0x2000):
                    context->stack[context->SP++] = context->PC;
                    context->PC = instruction & 0x0FFF;
                    break;
                case(0x3000):
                    context->PC = context->reg_V[(instruction & 0x0F00) >> 8] == (instruction & 0x00FF) ? context->PC + 2 : context->PC;
                    break;
                case(0x4000):
                    context->PC = context->reg_V[(instruction & 0x0F00) >> 8] != (instruction & 0x00FF) ? context->PC + 2 : context->PC;
                    break;
                case(0x5000):
                    context->PC = context->reg_V[(instruction & 0x0F00) >> 8] == context->reg_V[(instruction & 0x00F0) >> 4] ? context->PC + 2 : context->PC;
                    break;
                case (0x6000):
                    context->reg_V[(instruction & 0x0F00) >> 8] = instruction & 0x00FF;
                    break;
                case (0x7000):
                    context->reg_V[(instruction & 0x0F00) >> 8] += instruction & 0x00FF;
                    break;
                case (0x8000):
                    switch (instruction & 0xF00F)
                    {
                        case (0x8000):
                            context->reg_V[(instruction & 0x0F00) >> 8] = context->reg_V[(instruction & 0x00F0) >> 4];
                            break;
                        case (0x8001):
                            context->reg_V[(instruction & 0x0F00) >> 8] |= context->reg_V[(instruction & 0x00F0) >> 4];
                            if (context->version == VERSION_CHIP8) context->reg_V[0xF] = 0;
                            break;
                        case (0x8002):
                            context->reg_V[(instruction & 0x0F00) >> 8] &= context->reg_V[(instruction & 0x00F0) >> 4];
                            if (context->version == VERSION_CHIP8) context->reg_V[0xF] = 0;
                            break;
                        case (0x8003):
                            context->reg_V[(instruction & 0x0F00) >> 8] ^= context->reg_V[(instruction & 0x00F0) >> 4];
                            if (context->version == VERSION_CHIP8) context->reg_V[0xF] = 0;
                            break;
                        case (0x8004):
                            temp = ((uint8_t) (context->reg_V[(instruction & 0x0F00) >> 8] + context->reg_V[(instruction & 0x00F0) >> 4]))
                                                  < context->reg_V[(instruction & 0x0F00) >> 8];
                            context->reg_V[(instruction & 0x0F00) >> 8] += context->reg_V[(instruction & 0x00F0) >> 4];
                            context->reg_V[0xF] = temp;
                            break;
                        case (0x8005):
                            temp = context->reg_V[(instruction & 0x0F00) >> 8] >= context->reg_V[(instruction & 0x00F0) >> 4];
                            context->reg_V[(instruction & 0x0F00) >> 8] -= context->reg_V[(instruction & 0x00F0) >> 4];
                            context->reg_V[0xF] = temp;
                            break;
                        case (0x800E):
                            if (context->version == VERSION_CHIP8 || context->version == VERSION_CHIP8_MODERN)
                            {
                                temp = (context->reg_V[(instruction & 0x00F0) >> 4] & 0b10000000) >> 15;
                                context->reg_V[(instruction & 0x0F00) >> 8] = context->reg_V[(instruction & 0x00F0)>> 4];
                            }
                            else
                            {
                                temp = (context->reg_V[(instruction & 0x0F00) >> 8] & 0b10000000) != 0;
                            }
                            context->reg_V[(instruction & 0x0F00) >> 8] <<= 1;
                            context->reg_V[0xF] = temp;
                            break;
                        case (0x8007):
                            temp = context->reg_V[(instruction & 0x00F0) >> 4] >= context->reg_V[(instruction & 0x0F00) >> 8];
                            context->reg_V[(instruction & 0x0F00) >> 8] = context->reg_V[(instruction & 0x00F0) >> 4] - context->reg_V[(instruction & 0x0F00) >> 8];
                            context->reg_V[0xF] = temp;
                            break;
                        case (0x8006):
                            if (context->version == VERSION_CHIP8 || context->version == VERSION_CHIP8_MODERN)
                            {
                                temp = (context->reg_V[(instruction & 0x00F0) >> 4] & 0b00000001);
                                context->reg_V[(instruction & 0x0F00) >> 8] = context->reg_V[(instruction & 0x00F0)>> 4] >> 1;
                            }
                            if (context->version == VERSION_SUPERCHIP)
                            {
                                temp = (context->reg_V[(instruction & 0x0F00) >> 8] & 0b00000001);
                                context->reg_V[(instruction & 0x0F00) >> 8] >>= 1;
                            }
                            context->reg_V[0xF] = temp;
                            break;
                    }
                    break;
                case(0x9000):
                    context->PC = context->reg_V[(instruction & 0x0F00) >> 8] != context->reg_V[(instruction & 0x00F0) >> 4] ? context->PC + 2 : context->PC;
                    break;
                case (0xA000):
                    context->reg_I = instruction & 0x0FFF;
                    break;
                case (0xB000):
                    if (context->version == VERSION_CHIP8 || context->version == VERSION_CHIP8_MODERN)
                        context->PC = (instruction & 0x0FFF) + context->reg_V[0x0];
                    if (context->version == VERSION_SUPERCHIP)
                        context->PC = (instruction & 0x0FFF) + context->reg_V[instruction & 0x0F00 >> 8];
                    break;
                case (0xC000):
                    context->reg_V[(instruction & 0x0F00) >> 8] = (rand() % 0xFFFF) & (instruction & 0x00FF);
                    break;
                case (0xD000):
                    draw_sprite(context, instruction);
                    break;
                case (0xE000):
                    switch (instruction & 0xF0FF)
                    {
                        case (0xE09E):
                            context->PC = (context->keypad.keys >> context->reg_V[(instruction & 0x0F00) >> 8]) & 0x0001 ? context->PC + 2 : context->PC;
                            break;
                        case (0xE0A1):
                            context->PC = (context->keypad.keys >> context->reg_V[(instruction & 0x0F00) >> 8]) & 0x0001 ? context->PC : context->PC + 2;
                            break;
                    }
                    break;
                case (0xF000):
                    switch (instruction & 0xF0FF)
                    {
                        case (0xF007):
                            context->reg_V[(instruction & 0x0F00) >> 8] = context->reg_DT;
                            break;
                        case (0xF015):
                            context->reg_DT = context->reg_V[(instruction & 0x0F00) >> 8];
                            break;
                        case (0xF018):
                            context->reg_ST = context->reg_V[(instruction & 0x0F00) >> 8];
                            break;
                        case (0xF01E):
                            context->reg_I += context->reg_V[(instruction & 0x0F00) >> 8];
                            // context->reg_V[0xF] = context->reg_I >= 0x1000; amiga type of stuff, idk
                            break;
                        case (0xF00A):
                            pthread_mutex_lock(&context->mutex);
                            pthread_cond_wait(&context->haltCond, &context->mutex);
                            context->reg_V[(instruction & 0x0F00) >> 8] = context->keypad.last_key;
                            pthread_mutex_unlock(&context->mutex);
                            break;
                        case (0xF029):
                            context->reg_I = context->font_adresses[context->reg_V[(instruction & 0x0F00) >> 8] % 0x000F];
                            break;
                        case (0xF030):
                            if (context->version == VERSION_SUPERCHIP)
                                context->reg_I = context->big_font_adresses[context->reg_V[(instruction & 0x0F00) >> 8] % 0x0009];
                            break;
                        case (0xF033):
                            context->memory[context->reg_I] = (context->reg_V[(instruction & 0x0F00) >> 8]) / 100;
                            context->memory[context->reg_I + 1] = ((context->reg_V[(instruction & 0x0F00) >> 8]) / 10) % 10;
                            context->memory[context->reg_I + 2] = (context->reg_V[(instruction & 0x0F00) >> 8]) % 10;
                            break;
                        case (0xF055):
                            if (context->version == VERSION_CHIP8 || context->version == VERSION_CHIP8_MODERN)
                            {
                                for (int i = 0; i < ((instruction & 0x0F00) >> 8) + 1; i++)
                                {
                                    context->memory[context->reg_I] = context->reg_V[i];
                                    context->reg_I++;
                                }
                            }
                            if (context->version == VERSION_SUPERCHIP)
                            {
                                for (int i = 0; i < ((instruction & 0x0F00) >> 8) + 1; i++)
                                {
                                    context->memory[context->reg_I + i] = context->reg_V[i];
                                }
                            }
                            break;
                        case (0xF065):
                            if (context->version == VERSION_CHIP8 || context->version == VERSION_CHIP8_MODERN)
                            {
                                for (int i = 0; i < ((instruction & 0x0F00) >> 8) + 1; i++)
                                {
                                    context->reg_V[i] = context->memory[context->reg_I];
                                    context->reg_I++;
                                }
                            }
                            else
                            {
                                for (int i = 0; i < ((instruction & 0x0F00) >> 8) + 1; i++)
                                {
                                    context->reg_V[i] = context->memory[context->reg_I + i];
                                }
                            }
                            break;
                    }
                    break;
                default:
                    printf("Unsupported Instruction: %x\n", instruction);
                    break;
            }
    }
}