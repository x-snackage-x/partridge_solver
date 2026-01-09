#include <elhaylib.h>
#include <stdbool.h>

typedef struct block_def {
    int size;
    int free_pieces;
} block_def;

typedef struct puzzle_def {
    int size;
    int grid_dimension;
    int** puzzle_grid;
    dynarr_head* blocks;
} puzzle_def;

typedef enum RETURN_CODES {
    SUCCESS,
    NO_FREE_PIECES,
    CONFLICT_ON_GRID
} RETURN_CODES;

void init_puzzle(puzzle_def* puzzle_def);
RETURN_CODES place_block(puzzle_def* puzzle_def,
                         int block_id,
                         int x_pos,
                         int y_pos);
int get_n_available_pieces(puzzle_def* puzzle_def, int block_id);
bool placement_conflicts(puzzle_def* puzzle_def,
                         int block_id,
                         int x_pos,
                         int y_pos);
bool is_puzzle_solved(puzzle_def* puzzle_def);