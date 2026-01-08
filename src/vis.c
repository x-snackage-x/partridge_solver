#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#include <unistd.h>
#endif

#include <vis.h>
#define STEP_SIZE 2

void set_block(int block_size, COLOR color, int x_pos, int y_pos) {
    printf("\x1b[s");

    int x_shift = x_pos != 0 ? STEP_SIZE * x_pos + 1 : x_pos;
    int y_shift = y_pos;
    if(x_shift != 0) {
        printf("\x1b[%dG", x_shift);
    }
    if(y_shift != 0) {
        printf("\x1b[%dB", y_shift);
    }

    for(int i = 0; i < block_size; ++i) {
        for(int j = 0; j < block_size; ++j) {
            printf("\x1b[38;5;%dm\x1b[48;5;%dm▀\x1b[0m", color, color);
            printf("\x1b[38;5;%dm\x1b[48;5;%dm▀\x1b[0m", color, color);
        }
        printf("\x1b[1B\x1b[%dG", x_shift);
    }
    printf("\x1b[u");
}

void render_grid(int size) {
    printf("\x1b[%dB", size);
    fflush(stdout);
}

void prep_grid(int size) {
    for(int line = 0; line < size; ++line) {
#ifdef TESTING
        for(int i = 0; i < STEP_SIZE * size; i++) {
            putchar('.');
        }
        putchar('\n');
#else
        printf("%*c\n", STEP_SIZE * size, ' ');
#endif
    }

    printf("\x1b[%dA", size);
}

void reset_grid(int size) {
    printf("\x1b[%dA", size);
}

int main() {
    int grid_size = 45;

    prep_grid(grid_size);
    set_block(2, GREEN, 1, 0);
    set_block(1, WHITE, 0, 0);
    set_block(2, ROYAL_BLUE, 5, 2);
    set_block(5, GRAY, 5, 5);
    render_grid(grid_size);
    reset_grid(grid_size);

    usleep(25 * 100000);

    prep_grid(grid_size);
    set_block(2, GREEN, 1, 0);
    set_block(1, WHITE, 0, 0);
    set_block(2, BLUE, 5, 2);
    render_grid(grid_size);
    reset_grid(grid_size);

    usleep(25 * 100000);

    prep_grid(grid_size);
    set_block(2, GREEN, 1, 0);
    set_block(1, WHITE, 0, 0);
    set_block(2, BLUE, 5, 2);
    set_block(5, GRAY, 5, 5);
    render_grid(grid_size);

    usleep(2 * 1000000);
}