// First check if puzzle is in solvable state
// - Find smallest, bounded gap in all line scans
//     * Horizontally and Vertically
// - If smaller than available smallest piece
// -> Puzzle not in solvable state and go back in tree
// Else place tile

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <puz.h>
#include <sol.h>

VIS_F_PTR grid_prep_func;
VIS_F_PTR grid_render_func;
VIS_F_PTR grid_reset_func;
VIS_SET_F_PTR bloc_set_func;

puzzle_def* my_puzzle;

tree_head placement_record;
tree_op_res tree_result;
size_t node_size;
tree_node* last_placement;

typedef struct {
    int tile_type;
    int x_pos;
    int y_pos;
    bool valid_tiles[];
} node_placement;

typedef enum { NODE_PARTRIDGE = 1001 } my_node_types;

int random_tile_select(bool* filter, int max_tile_size);
bool line_scan_hor(puzzle_def* puzzle, point* result);
bool find_smallest_gap(puzzle_def* puzzle, gap_search_result* res_struct);

void setup(int puzzle_type) {
    my_puzzle = calloc(1, sizeof(puzzle_def));

    my_puzzle->size = puzzle_type;
    init_puzzle(my_puzzle);

    node_size = sizeof(node_placement) + sizeof(bool) * (puzzle_type + 1);
    tree_init(&placement_record);

    // place root node
    bool* valid_tiles = malloc(sizeof(bool) * (puzzle_type + 1));
    memset(valid_tiles, true, sizeof(bool) * (puzzle_type + 1));
    valid_tiles[0] = false;
    valid_tiles[1] = false;
    valid_tiles[2] = false;
    valid_tiles[3] = false;
    valid_tiles[4] = false;
    int selected_tile = random_tile_select(valid_tiles, puzzle_type);

    node_placement node_buffer = {
        .tile_type = selected_tile, .x_pos = 0, .y_pos = 0};
    // memcpy(node_buffer.valid_tiles, valid_tiles,
    //        sizeof(bool) * (puzzle_type + 1));
    tree_node_root(&tree_result, &placement_record, NODE_PARTRIDGE, node_size,
                   &node_buffer);
    last_placement = tree_result.node_ptr;
    // free(valid_tiles);

    srand(time(NULL));
}

tree_node* record_placement(int tile_type,
                            int x_pos,
                            int y_pos,
                            bool* valid_tiles,
                            tree_node* prev_placement) {
    node_placement node_buffer = {
        .tile_type = tile_type, .x_pos = x_pos, .y_pos = y_pos};
    memcpy(node_buffer.valid_tiles, valid_tiles,
           sizeof(bool) * (my_puzzle->size + 1));

    tree_node_add(&tree_result, &placement_record, prev_placement,
                  NODE_PARTRIDGE, node_size, &node_buffer);

    return tree_result.node_ptr;
}

int main() {
    int puzzle_type = 8;
    setup(puzzle_type);

    bool is_solvable = is_solvable_first_check(my_puzzle);
    bool is_solved = is_puzzle_solved(my_puzzle);

    bool* valid_tiles = malloc(sizeof(bool) * (puzzle_type + 1));
    memset(valid_tiles, true, sizeof(bool) * (puzzle_type + 1));
    valid_tiles[0] = false;
    valid_tiles[1] = false;

    point result_buffer;
    int loop_n = 0;
    while(is_solvable && !is_solved && loop_n++ < 30) {
        line_scan_hor(my_puzzle, &result_buffer);

        int selected_tile = random_tile_select(valid_tiles, puzzle_type);
        printf("Current tile: %d\n", selected_tile);
        if(placement_resolvable(my_puzzle, selected_tile, result_buffer.x_index,
                                result_buffer.y_index)) {
            place_block(my_puzzle, selected_tile, result_buffer.x_index,
                        result_buffer.y_index);
            // valid_tiles[selected_tile] = false;
        }

        is_solvable = is_solvable_first_check(my_puzzle);
        is_solved = is_puzzle_solved(my_puzzle);
    }
    printf("Puzzle Status: Solvable: %s - Solved: %s\n",
           is_solvable ? "true" : "false", is_solved ? "true" : "false");
    print_grid(my_puzzle);
}

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
bool find_smallest_gap(puzzle_def* puzzle, gap_search_result* res_struct) {
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
        res_struct->type = VERTICAL;
    }

    return true;
}

bool is_solvable_first_check(puzzle_def* puzzle) {
    gap_search_result result;
    find_smallest_gap(puzzle, &result);

    bool is_solvabe = true;
    if(result.gap <= puzzle->size) {
        is_solvabe = (get_n_available_pieces(puzzle, result.gap) > 0);
    }

    return is_solvabe;
}

int random_tile_select(bool* filter, int max_tile_size) {
    int* candidate_tiles = calloc((size_t)max_tile_size, sizeof(int));
    int j = 0;
    for(int i = 1; i <= max_tile_size; ++i) {
        if(filter[i]) {
            candidate_tiles[j++] = i;
        }
    }

    bool tile_not_found = true;
    int random_tile = 0;
    do {
        random_tile = rand() % max_tile_size;
        for(int i = 0; i < j && tile_not_found; ++i) {
            if(candidate_tiles[i] == random_tile) {
                tile_not_found = false;
            }
        }
    } while(tile_not_found);
    free(candidate_tiles);

    return random_tile;
}

int largest_tile_select(bool* filter, int max_tile_size) {
    int selected_tile = 0;
    for(int i = max_tile_size; i > 0; --i) {
        if(filter[i]) {
            selected_tile = i;
            break;
        }
    }

    return selected_tile;
}