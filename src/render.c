#include <err.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "render.h"
#include "debugger.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240
#define SCREEN_SCALE 4

bool initialized = false;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

button player1_buttons[8];

color last_screen[SCREEN_WIDTH][SCREEN_HEIGHT];

void initialize() {
    initialized = true;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        errx(EXIT_FAILURE, "SDL couldn't initialize! %s", SDL_GetError());
    }
    window = SDL_CreateWindow("dgb nes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * SCREEN_SCALE, SCREEN_HEIGHT * SCREEN_SCALE, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (renderer == NULL) {
        errx(EXIT_FAILURE, "SDL couldn't create a renderer! %s", SDL_GetError());
    }

    SDL_RenderSetScale(renderer, SCREEN_SCALE, SCREEN_SCALE);

    for (int i = 0; i < 8; i++) {
        player1_buttons[i] = false;
    }

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            color c;
            c.r = 0;
            c.g = 0;
            c.b = 0;
            c.a = 0;
            last_screen[x][y] = c;
        }
    }
}

void update_key(SDL_Keycode sdlk, bool state) {
    switch (sdlk) {
        case SDLK_ESCAPE:
            errx(EXIT_SUCCESS, "User pressed escape");

        case SDLK_UP:
        case SDLK_w:
            player1_buttons[UP] = state;
            break;
        case SDLK_s:
        case SDLK_DOWN:
            player1_buttons[DOWN] = state;
            break;
        case SDLK_a:
        case SDLK_LEFT:
            player1_buttons[LEFT] = state;
            break;
        case SDLK_d:
        case SDLK_RIGHT:
            player1_buttons[RIGHT] = state;
            break;
        case SDLK_z:
        case SDLK_q:
        case SDLK_j:
            player1_buttons[A] = state;
            break;
        case SDLK_x:
        case SDLK_e:
        case SDLK_k:
            player1_buttons[B] = state;
            break;
        case SDLK_RETURN:
            player1_buttons[START] = state;
            break;
        case SDLK_RSHIFT:
            player1_buttons[SELECT] = state;
            return;
        default:
            break;
    }
}

void render_screen(color (*screen)[SCREEN_WIDTH][SCREEN_HEIGHT]) {
    if (!initialized) {
        initialize();
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                errx(EXIT_FAILURE, "User requested quit");
            case SDL_KEYDOWN:
                update_key(event.key.keysym.sym, true);
                break;
            case SDL_KEYUP:
                update_key(event.key.keysym.sym, false);
                break;
            default:
                break;
        }
    }


    // TODO this is probably kinda slow
    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 256; x++) {
            color c = (*screen)[x][y];
            color lastc = last_screen[x][y];
            if (c.r == lastc.r && c.g == lastc.g && c.b == lastc.b) {
                continue;
            }
            last_screen[x][y] = c;

            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    dprintf("Updating renderer\n");
    SDL_RenderPresent(renderer);
}

bool get_button(button btn, player p) {
    if (p == one) {
        return player1_buttons[btn];
    }
    else {
        return false;
    }
}

// Not called from anywhere yet
void cleanup() {
    //TODO cleanup SDL stuff
}
