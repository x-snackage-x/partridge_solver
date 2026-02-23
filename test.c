#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main() {
    srand(time(NULL));
    int puzzle_type = 8;
    int min_Tile = 5;
    if(puzzle_type <= 4) {
        min_Tile = 1;
    }
    int selected_tile = rand() % (puzzle_type + 1 - min_Tile) + min_Tile;

    int counts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    for(int i = 0; i < 100; ++i) {
        printf("Chosen root: %d\n", selected_tile);
        ++counts[selected_tile];

        selected_tile = rand() % (puzzle_type + 1 - min_Tile) + min_Tile;
    }
    printf("\n");

    printf("0. - %d times\n", counts[0]);
    printf("1. - %d times\n", counts[1]);
    printf("2. - %d times\n", counts[2]);
    printf("3. - %d times\n", counts[3]);
    printf("4. - %d times\n", counts[4]);
    printf("5. - %d times\n", counts[5]);
    printf("6. - %d times\n", counts[6]);
    printf("7. - %d times\n", counts[7]);
    printf("8. - %d times\n", counts[8]);
}