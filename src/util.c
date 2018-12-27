#include "util.h"

void wait_interactive() {
    printf("press enter\n");
    while (getchar() != '\n');
}
