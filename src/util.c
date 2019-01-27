#include <err.h>
#include <stdlib.h>

#include "util.h"

void wait_interactive() {
    printf("press enter\n");
    while (getchar() != '\n');
}


byte mask_flag(int index) {
    if (index > 7) {
        errx(EXIT_FAILURE, "Attempted to mask a flag > 7: %d. WTF?", index);
    }

    return 0b10000000 >> (7 - index);
}
