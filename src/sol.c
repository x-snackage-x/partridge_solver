// First check if puzzle is in solvable state
// - Find smallest, bounded gap in all line scans
//     * Horizontally and Vertically
// - If smaller than available smallest piece
// -> Puzzle not in solvable state and go back in tree
// Else place tile

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <puz.h>
#include <sol.h>

VIS_F_PTR grid_prep_func;
VIS_F_PTR grid_render_func;
VIS_F_PTR grid_reset_func;
VIS_SET_F_PTR bloc_set_func;

puzzle_def* my_puzzle;

typedef struct {
    int tile_type;
    int x_pos;
    int y_pos;
    bool valid_tiles[];
} node_placement;

bool line_scan_hor(puzzle_def* puzzle, point* result) {
    int** grid = puzzle->puzzle_grid;
    for(int i = 0; i < puzzle->grid_dimension; ++i) {
        for(int j = 0; j < puzzle->grid_dimension; ++j) {
            if(grid[i][j] == 0) {
                result->x_index = j;
                result->y_index = i;
                return true;
            }
        }
    }

    return false;
}

// - Find smallest, bounded gap in all line scans
//     * Horizontally + Vertically
bool find_smallets_gap(puzzle_def* puzzle, gap_search_result* res_struct) {
    res_struct->x_index = 0;
    res_struct->y_index = 0;
    res_struct->gap = 0;
    res_struct->type = NOT_FOUND;

    if(is_puzzle_solved(puzzle)) {
        return false;
    }

    int** grid = puzzle->puzzle_grid;

    // Horizontal Scan
    int x_index_hor_scan = 0;
    int y_index_hor_scan = 0;
    int size_hor_scan = puzzle->grid_dimension + 1;
    for(int i = 0; i < puzzle->grid_dimension; ++i) {
        int line_gap_count = 0;
        int index_buffer = 0;
        bool am_counting = false;
        for(int j = 0; j < puzzle->grid_dimension; ++j) {
            if(grid[i][j] == 0 && am_counting) {
                ++line_gap_count;
            } else if(grid[i][j] == 0 && !am_counting) {
                ++line_gap_count;
                index_buffer = j;
                am_counting = true;
            } else if((grid[i][j] != 0 ||
                       // Handle grid edge
                       j == puzzle->grid_dimension - 1) &&
                      am_counting) {
                if(size_hor_scan > line_gap_count) {
                    size_hor_scan = line_gap_count;
                    x_index_hor_scan = i;
                    y_index_hor_scan = index_buffer;
                }
                index_buffer = 0;
                am_counting = false;
            }
        }
        if(size_hor_scan > line_gap_count && am_counting) {
            size_hor_scan = line_gap_count;
            x_index_hor_scan = i;
            y_index_hor_scan = index_buffer;
        }
    }

    // Vertical Scan
    int x_index_ver_scan = 0;
    int y_index_ver_scan = 0;
    int size_ver_scan = puzzle->grid_dimension + 1;
    for(int j = 0; j < puzzle->grid_dimension; ++j) {
        int column_gap_count = 0;
        int index_buffer = 0;
        bool am_counting = false;
        for(int i = 0; i < puzzle->grid_dimension; ++i) {
            if(grid[i][j] == 0 && am_counting) {
                ++column_gap_count;
            } else if(grid[i][j] == 0 && !am_counting) {
                ++column_gap_count;
                index_buffer = i;
                am_counting = true;
            } else if((grid[i][j] != 0 ||
                       // Handle grid edge
                       j == puzzle->grid_dimension - 1) &&
                      am_counting) {
                if(size_ver_scan > column_gap_count) {
                    size_ver_scan = column_gap_count;
                    x_index_ver_scan = index_buffer;
                    y_index_ver_scan = j;
                }
                index_buffer = 0;
                am_counting = false;
            }
        }
        if(size_ver_scan > column_gap_count && am_counting) {
            size_ver_scan = column_gap_count;
            x_index_ver_scan = index_buffer;
            y_index_ver_scan = j;
        }
    }

    if(size_hor_scan <= size_ver_scan) {
        res_struct->x_index = x_index_hor_scan;
        res_struct->y_index = y_index_hor_scan;
        res_struct->gap = size_hor_scan;
        res_struct->type = HORIZONTAL;
    } else {
        res_struct->x_index = x_index_ver_scan;
        res_struct->y_index = y_index_ver_scan;
        res_struct->gap = size_ver_scan;
        res_struct->type = VERTIICAL;
    }

    return true;
}

void setup(int puzzle_type) {
    my_puzzle = calloc(1, sizeof(puzzle_def));

    my_puzzle->size = puzzle_type;
    init_puzzle(my_puzzle);
}

bool is_solvable_first_check(puzzle_def* puzzle) {
    gap_search_result result;
    find_smallets_gap(puzzle, &result);

    return get_n_available_pieces(puzzle, result.gap) > 0;
}

int main() {
    int puzzle_type = 8;
    setup(puzzle_type);

    bool is_solvable = is_solvable_first_check(my_puzzle);
    bool is_solved = is_puzzle_solved(my_puzzle);

    bool* valid_tiles = malloc(sizeof(bool) * puzzle_type + 1);
    memset(valid_tiles, true, sizeof(bool) * puzzle_type + 1);
    valid_tiles[0] = false;

    point result_buffer;
    bool tiles_exhausted = false;
    while(is_solvable && !is_solved && !tiles_exhausted) {
        line_scan_hor(my_puzzle, &result_buffer);

        int largest_valid_tile;
        for(int i = puzzle_type; i > 0; --i) {
            if(valid_tiles[i]) {
                largest_valid_tile = i;
                break;
            }
        }

        for(int i = largest_valid_tile; i > 0; --i) {
            if(placement_conflicts(my_puzzle, i, result_buffer.x_index,
                                   result_buffer.y_index)) {
                place_block(my_puzzle, i, result_buffer.x_index,
                            result_buffer.y_index);
                valid_tiles[i] = false;
                break;
            }
        }

        is_solvable = is_solvable_first_check(my_puzzle);
        is_solved = is_puzzle_solved(my_puzzle);
    }
    printf("Puzzle Status: Solvable: %s - Solved: %s\n",
           is_solvable ? "true" : "false", is_solved ? "true" : "false");
    print_grid(my_puzzle);
}