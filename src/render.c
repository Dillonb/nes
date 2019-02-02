#include <err.h>
#include <stdlib.h>

#include "render.h"

void render_screen(color screen[256][240]) {
    // SUPER TEMPORARY (obviously)
    printf("\n\n\n");
    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 256; x++) {
            color c = screen[x][y];
            switch (c.r) {
                case 0x0:
                    printf("%%");
                    break;
                case 0x55:
                    printf("o");
                    break;
                case 0xAA:
                    printf(".");
                    break;
                case 0xFF:
                    printf(" ");
                    break;
                default:
                    printf("Saw a color I didn't recognize\n");
                    return;
                    
            }
        }
        printf("\n");
    }
    printf("\n\n\n");
}
