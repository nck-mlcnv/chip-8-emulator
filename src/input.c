#include "input.h"


void handle_input(System_Context* context, SDL_Event* event)
{
    if (event->type == SDL_KEYDOWN)
    {
        switch (event->key.keysym.scancode)
        {
            case (SDL_SCANCODE_1):
                context->keypad.keys |= 0b0000000000000010;
                break;
            case (SDL_SCANCODE_2):
                context->keypad.keys |= 0b0000000000000100;
                break;
            case (SDL_SCANCODE_3):
                context->keypad.keys |= 0b0000000000001000;
                break;
            case (SDL_SCANCODE_4):
                context->keypad.keys |= 0b0001000000000000;
                break;
            case (SDL_SCANCODE_Q):
                context->keypad.keys |= 0b0000000000010000;
                break;
            case (SDL_SCANCODE_W):
                context->keypad.keys |= 0b0000000000100000;
                break;
            case (SDL_SCANCODE_E):
                context->keypad.keys |= 0b0000000001000000;
                break;
            case (SDL_SCANCODE_R):
                context->keypad.keys |= 0b0010000000000000;
                break;
            case (SDL_SCANCODE_A):
                context->keypad.keys |= 0b0000000010000000;
                break;
            case (SDL_SCANCODE_S):
                context->keypad.keys |= 0b0000000100000000;
                break;
            case (SDL_SCANCODE_D):
                context->keypad.keys |= 0b0000001000000000;
                break;
            case (SDL_SCANCODE_F):
                context->keypad.keys |= 0b0100000000000000;
                break;
            case (SDL_SCANCODE_Z):
                context->keypad.keys |= 0b0000010000000000;
                break;
            case (SDL_SCANCODE_X):
                context->keypad.keys |= 0b0000000000000001;
                break;
            case (SDL_SCANCODE_C):
                context->keypad.keys |= 0b0000100000000000;
                break;
            case (SDL_SCANCODE_V):
                context->keypad.keys |= 0b1000000000000000;
                break;
            default:
                break;
        }
    }
    else
    {
        switch (event->key.keysym.scancode)
        {
            case (SDL_SCANCODE_1):
                context->keypad.keys &= 0b1111111111111101;
                context->keypad.last_key = 0x01;
                break;
            case (SDL_SCANCODE_2):
                context->keypad.keys &= 0b1111111111111011;
                context->keypad.last_key = 0x02;
                break;
            case (SDL_SCANCODE_3):
                context->keypad.keys &= 0b1111111111110111;
                context->keypad.last_key = 0x03;
                break;
            case (SDL_SCANCODE_4):
                context->keypad.keys &= 0b1110111111111111;
                context->keypad.last_key = 0x0C;
                break;
            case (SDL_SCANCODE_Q):
                context->keypad.keys &= 0b1111111111101111;
                context->keypad.last_key = 0x04;
                break;
            case (SDL_SCANCODE_W):
                context->keypad.keys &= 0b1111111111011111;
                context->keypad.last_key = 0x05;
                break;
            case (SDL_SCANCODE_E):
                context->keypad.keys &= 0b1111111110111111;
                context->keypad.last_key = 0x06;
                break;
            case (SDL_SCANCODE_R):
                context->keypad.keys &= 0b1101111111111111;
                context->keypad.last_key = 0x0D;
                break;
            case (SDL_SCANCODE_A):
                context->keypad.keys &= 0b1111111101111111;
                context->keypad.last_key = 0x07;
                break;
            case (SDL_SCANCODE_S):
                context->keypad.keys &= 0b1111111011111111;
                context->keypad.last_key = 0x08;
                break;
            case (SDL_SCANCODE_D):
                context->keypad.keys &= 0b1111110111111111;
                context->keypad.last_key = 0x09;
                break;
            case (SDL_SCANCODE_F):
                context->keypad.keys &= 0b1011111111111111;
                context->keypad.last_key = 0x0E;
                break;
            case (SDL_SCANCODE_Z):
                context->keypad.keys &= 0b1111101111111111;
                context->keypad.last_key = 0x0A;
                break;
            case (SDL_SCANCODE_X):
                context->keypad.keys &= 0b1111111111111110;
                context->keypad.last_key = 0x00;
                break;
            case (SDL_SCANCODE_C):
                context->keypad.keys &= 0b1111011111111111;
                context->keypad.last_key = 0x0B;
                break;
            case (SDL_SCANCODE_V):
                context->keypad.keys &= 0b0111111111111111;
                context->keypad.last_key = 0x0F;
                break;
            default:
                break;
        }
        pthread_cond_broadcast(&context->haltCond);
    }
}