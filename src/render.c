#include <err.h>
#include <stdlib.h>
#include <SDL.h>
#include <stdbool.h>

#include "render.h"
#include "debugger.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240
#define SCREEN_SCALE 4

bool initialized = false;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;


void initialize() {
    initialized = true;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        errx(EXIT_FAILURE, "SDL couldn't initialize! %s", SDL_GetError());
    }
    window = SDL_CreateWindow("dgb nes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * SCREEN_SCALE, SCREEN_HEIGHT * SCREEN_SCALE, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        errx(EXIT_FAILURE, "SDL couldn't create a renderer! %s", SDL_GetError());
    }
}

void render_screen(color screen[SCREEN_WIDTH][SCREEN_HEIGHT]) {
    if (!initialized) {
        initialize();
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                errx(EXIT_FAILURE, "User requested quit");
            default:
                break;
        }
    }


    // TODO this is probably kinda slow
    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 256; x++) {
            color c = screen[x][y];
            SDL_Rect rect;
            rect.x = x * SCREEN_SCALE;
            rect.y = y * SCREEN_SCALE;
            rect.h = SCREEN_SCALE;
            rect.w = SCREEN_SCALE;

            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    printf("Updating renderer\n");
    SDL_RenderPresent(renderer);
}


// Not called from anywhere yet
void cleanup() {
    //TODO cleanup SDL stuff
}
