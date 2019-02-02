#include <err.h>
#include <stdlib.h>
#include <SDL.h>
#include <stdbool.h>

#include "render.h"
#include "debugger.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

bool initialized = false;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;
byte* pixels = NULL;


void initialize() {
    initialized = true;
    pixels = malloc(4 * SCREEN_WIDTH * SCREEN_HEIGHT);
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        errx(EXIT_FAILURE, "SDL couldn't initialize! %s", SDL_GetError());
    }
    window = SDL_CreateWindow("dgb nes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        errx(EXIT_FAILURE, "SDL couldn't create a renderer! %s", SDL_GetError());
    }
}

void render_screen(color screen[SCREEN_WIDTH][SCREEN_HEIGHT]) {
    if (!initialized) {
        initialize();
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                errx(EXIT_FAILURE, "User requested quit");
            default:
                break;
        }
    }


    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 256; x++) {
            color c = screen[x][y];

            int index = 4 * (x + y * SCREEN_WIDTH);

            pixels[index] = c.b;
            pixels[index + 1] = c.g;
            pixels[index + 2] = c.r;
            pixels[index + 3] = c.a;
        }
    }
    printf("Updating texture\n");
    SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * 4);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}


// Not called from anywhere yet
void cleanup() {
    free(pixels);
}
