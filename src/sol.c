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
#ifdef __linux__
#include <unistd.h>
#endif

#include <signal.h>
#include <sys/stat.h>

#include <puz.h>
#include <sol.h>
#include <vis.h>

FILE* log_fptr;
FILE* tree_fptr;

bool print_full_log;
bool visualizer_set;
VIS_F_PTR grid_prep_func;
VIS_F_PTR grid_render_func;
VIS_F_PTR grid_reset_func;
VIS_F_PTR grid_record_func;
VIS_SET_F_PTR block_set_func;
VIS_SET_F_PTR block_remove_func;
VIS_SET_C_PTR block_set_color_func;

puzzle_def* my_puzzle;

tree_head placement_record;
tree_op_res tree_result;
size_t node_size;
tree_node* last_placement;

int root_tile;

// the bool valid_tiles[] array describes all the tiles
// that have been attempted as children
// if it is exhausted then the tree descent has to move
// to the parent of the selected node
typedef struct {
    int tile_type;
    int x_pos;
    int y_pos;
    bool valid_tiles[17];
} node_placement;

bool is_solvable;
bool is_solved;

typedef enum { NODE_PARTRIDGE = 1001 } my_node_types;

int random_tile_select(bool* filter, int max_tile_size);
int largest_tile_select(bool* filter, int max_tile_size);

bool line_scan_hor(puzzle_def* puzzle, point* result);
bool find_smallest_gap(puzzle_def* puzzle, gap_search_result* res_struct);
bool is_solvable_first_check(puzzle_def* puzzle);

int n_available_tiles(bool* valid_tiles);

void printNode(tree_node* ptr_node, FILE* file_ptr);
void printTree(tree_node* ptr_node,
               int depth,
               bool isLast,
               bool* flag,
               FILE* file_ptr);

void setup(int puzzle_type) {
    srand(time(NULL));

    my_puzzle = calloc(1, sizeof(puzzle_def));

    my_puzzle->size = puzzle_type;
    init_puzzle(my_puzzle);

    node_size = sizeof(node_placement);
    tree_init(&placement_record);

    // TODO change to dummy node
    // place root node
    int min_Tile = 5;
    if(puzzle_type <= 4) {
        min_Tile = 1;
    }
    int selected_tile = rand() % (puzzle_type + 1 - min_Tile) + min_Tile;
    bool* valid_tiles = malloc(sizeof(bool) * (puzzle_type + 1));
    memset(valid_tiles, true, sizeof(bool) * (puzzle_type + 1));
    valid_tiles[0] = false;

    point result_buffer = {0};
    place_block(my_puzzle, selected_tile, result_buffer.x_index,
                result_buffer.y_index);
    if(print_full_log)
        fprintf(log_fptr, "Placed Root tile: %d\n", selected_tile);

    node_placement* node_buffer = malloc(node_size);
    node_buffer->tile_type = selected_tile;
    node_buffer->x_pos = 0;
    node_buffer->y_pos = 0;
    memcpy(node_buffer->valid_tiles, valid_tiles,
           sizeof(bool) * (puzzle_type + 1));

    tree_node_root(&tree_result, &placement_record, NODE_PARTRIDGE, node_size,
                   node_buffer);
    last_placement = tree_result.node_ptr;
    free(valid_tiles);
    free(node_buffer);
    root_tile = selected_tile;
}

void set_visualizer(VIS_F_PTR grid_prep_func_in,
                    VIS_F_PTR grid_render_func_in,
                    VIS_F_PTR grid_reset_func_in,
                    VIS_F_PTR grid_record_func_in,
                    VIS_SET_F_PTR block_set_func_in,
                    VIS_SET_F_PTR block_remove_func_in,
                    VIS_SET_C_PTR block_set_color_func_in) {
    grid_prep_func = grid_prep_func_in;
    grid_render_func = grid_render_func_in;
    grid_reset_func = grid_reset_func_in;
    grid_record_func = grid_record_func_in;
    block_set_func = block_set_func_in;
    block_remove_func = block_remove_func_in;
    block_set_color_func = block_set_color_func_in;

    visualizer_set = true;
}

tree_node* record_placement(int selected_tile,
                            int x_pos,
                            int y_pos,
                            tree_node* prev_placement) {
    node_placement* node_buffer = malloc(node_size);
    node_buffer->tile_type = selected_tile;
    node_buffer->x_pos = x_pos;
    node_buffer->y_pos = y_pos;
    memset(node_buffer->valid_tiles, true,
           sizeof(bool) * (my_puzzle->size + 1));
    node_buffer->valid_tiles[0] = false;

    tree_node_add(&tree_result, &placement_record, prev_placement,
                  NODE_PARTRIDGE, node_size, node_buffer);
    free(node_buffer);

    // visualize placement
    if(visualizer_set) {
        block_set_func(selected_tile, x_pos, y_pos);
        grid_render_func(my_puzzle->grid_dimension);
        grid_reset_func(my_puzzle->grid_dimension);
        if(my_puzzle->size < 8)
            usleep(5 * 1000);
    }

    return tree_result.node_ptr;
}

bool* record_removal(int selected_tile,
                     int x_pos,
                     int y_pos,
                     tree_node* parent) {
    node_placement* parent_placement_data = (node_placement*)parent->data;
    bool* valid_tiles = parent_placement_data->valid_tiles;
    valid_tiles[selected_tile] = false;

    // visualize removal
    if(visualizer_set) {
        block_remove_func(selected_tile, x_pos, y_pos);
        grid_render_func(my_puzzle->grid_dimension);
        grid_reset_func(my_puzzle->grid_dimension);
        if(my_puzzle->size < 8)
            usleep(5 * 1000);
    }

    return valid_tiles;
}

bool solution_search() {
    int puzzle_type = my_puzzle->size;

    is_solvable = is_solvable_first_check(my_puzzle);
    is_solved = is_puzzle_solved(my_puzzle);

    bool* valid_tiles;

    point result_buffer = {0};
    int loop_n = 0;
    int max_loops = 100 * 1000 * 1000;
    while(!is_solved && loop_n++ < max_loops) {
        if(loop_n % 100000 == 0 && !visualizer_set) {
            printf("Current iter.: %d/%d", loop_n, max_loops);
            fflush(stdout);
            printf("\r");
        }

        line_scan_hor(my_puzzle, &result_buffer);

        node_placement* placement_data = (node_placement*)last_placement->data;
        valid_tiles = (bool*)&placement_data->valid_tiles;

        // select one tile and place
        RETURN_CODES placement_code = CONFLICT_ON_GRID;
        bool tiles_available = true;
        do {
            int selected_tile = random_tile_select(valid_tiles, puzzle_type);
            if(print_full_log)
                fprintf(log_fptr, "Current tile: %d", selected_tile);

            placement_code =
                place_block(my_puzzle, selected_tile, result_buffer.x_index,
                            result_buffer.y_index);

            if(placement_code == SUCCESS) {
                if(print_full_log)
                    fprintf(log_fptr, " - Placement success: true\n");

                last_placement =
                    record_placement(selected_tile, result_buffer.x_index,
                                     result_buffer.y_index, last_placement);

                // TODO set valid tile to false if tile is exhausted with
                // last placement
            } else {
                valid_tiles[selected_tile] = false;
                if(print_full_log)
                    fprintf(log_fptr, " - Placement success: false\n");
            }

            int available_tiles = n_available_tiles(valid_tiles);
            tiles_available = available_tiles > 0;

        } while(placement_code != SUCCESS && tiles_available);

        is_solvable = is_solvable_first_check(my_puzzle);
        if(!is_solvable) {
            tree_node* parent = last_placement->parent;
            node_placement cur_placement_data =
                *(node_placement*)last_placement->data;

            remove_block(my_puzzle, cur_placement_data.tile_type,
                         cur_placement_data.x_pos, cur_placement_data.y_pos);

            valid_tiles = record_removal(cur_placement_data.tile_type,
                                         cur_placement_data.x_pos,
                                         cur_placement_data.y_pos, parent);

            if(print_full_log)
                fprintf(log_fptr, " Remove tile: %d, Pos. (%2d,%2d)\n",
                        cur_placement_data.tile_type, cur_placement_data.x_pos,
                        cur_placement_data.y_pos);

            last_placement = parent;
        }

        int available_tiles = n_available_tiles(valid_tiles);
        tiles_available = available_tiles > 0;

        while(!tiles_available) {
            tree_node* parent = last_placement->parent;
            node_placement cur_placement_data =
                *(node_placement*)last_placement->data;

            remove_block(my_puzzle, cur_placement_data.tile_type,
                         cur_placement_data.x_pos, cur_placement_data.y_pos);

            if(last_placement == placement_record.tree_root) {
                if(print_full_log)
                    fprintf(log_fptr,
                            " Remove tile: %d, Pos. (%2d,%2d) - Root\n\n",
                            cur_placement_data.tile_type,
                            cur_placement_data.x_pos, cur_placement_data.y_pos);

                if(visualizer_set) {
                    block_remove_func(cur_placement_data.tile_type,
                                      cur_placement_data.x_pos,
                                      cur_placement_data.y_pos);
                    grid_render_func(my_puzzle->grid_dimension);
                    grid_reset_func(my_puzzle->grid_dimension);
                }

                goto finish;
            }

            valid_tiles = record_removal(cur_placement_data.tile_type,
                                         cur_placement_data.x_pos,
                                         cur_placement_data.y_pos, parent);

            if(print_full_log)
                fprintf(log_fptr, " Remove tile: %d, Pos. (%2d,%2d)\n",
                        cur_placement_data.tile_type, cur_placement_data.x_pos,
                        cur_placement_data.y_pos);

            last_placement = parent;

            available_tiles = n_available_tiles(valid_tiles);
            tiles_available = available_tiles > 0;
        }

        is_solved = is_puzzle_solved(my_puzzle);
    }

    if(loop_n == max_loops) {
        printf("Loop exhausted\n");
        if(print_full_log)
            fprintf(log_fptr, "Loop exhausted\n");
        return is_solvable;
    }

finish:
    return is_puzzle_solved(my_puzzle);
}

int main() {
    print_full_log = false;

    // Make logs dir
    struct stat st = {0};
    if(stat("logs", &st) == -1) {
        mkdir("logs", 0700);
    }

    // Open a file in writing mode
    log_fptr = fopen("logs/log.txt", "w");
    tree_fptr = fopen("logs/tree.txt", "w");

    int puzzle_type = 8;
    setup(puzzle_type);

    set_visualizer(prep_vis_grid, render_vis_grid, reset_vis_grid,
                   record_vis_grid, set_vis_block, remove_vis_block,
                   def_block_colors);
    if(visualizer_set) {
        COLOR blocks[] = {WHITE, ROYAL_BLUE, ORANGE, MAGENTA, CYAN,
                          RED,   GREEN,      GRAY,   YELLOW,  BLACK};
        block_set_color_func((int*)blocks, my_puzzle->size);
        grid_prep_func(my_puzzle->grid_dimension);

        // record root tile
        block_set_func(root_tile, 0, 0);
        grid_render_func(my_puzzle->grid_dimension);
        grid_reset_func(my_puzzle->grid_dimension);
        usleep(5 * 1000);
    }

    clock_t begin = clock();

    is_solvable = solution_search();

    clock_t end = clock();
    double solve_time = (double)(end - begin) / CLOCKS_PER_SEC;

    if(is_solved && visualizer_set) {
        grid_record_func(my_puzzle->grid_dimension);
    }

    fprintf(log_fptr, "Puzzle Status: Solvable: %s - Solved: %s\n\n",
            is_solvable ? "true" : "false", is_solved ? "true" : "false");
    printf("Puzzle Status: Solvable: %s - Solved: %s\n",
           is_solvable ? "true" : "false", is_solved ? "true" : "false");
    printf("\33[2K\r\n");

    print_grid(my_puzzle, NULL);
    printf("\n");
    print_free_pieces(my_puzzle, NULL);
    print_grid(my_puzzle, log_fptr);
    fprintf(log_fptr, "\n");
    print_free_pieces(my_puzzle, log_fptr);

    printf("\nTree Size: %zu Nodes\n", placement_record.tree_size);
    fprintf(tree_fptr, "\nTree Size: %zu Nodes\n", placement_record.tree_size);
    fprintf(log_fptr, "Tree Size: %zu Nodes\n", placement_record.tree_size);

    printf("Solve Time: %f seconds\n", solve_time);
    fprintf(log_fptr, "Solve Time: %f seconds\n", solve_time);

    bool* flags = malloc(sizeof(bool) * placement_record.tree_size);
    memset(flags, true, placement_record.tree_size);

    if(placement_record.tree_size <= 100000)
        printTree(placement_record.tree_root, 0, false, flags, tree_fptr);

    // Close the file
    fclose(log_fptr);
    fclose(tree_fptr);
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
    bool gap_bool = find_smallest_gap(puzzle, &result);

    int smallest_available_tile = 0;
    for(int i = puzzle->size; i > 0; --i) {
        int n_tiles_i = get_n_available_pieces(puzzle, i);
        if(n_tiles_i > 0)
            smallest_available_tile = i;
    }

    bool is_solvable = true;
    if(!gap_bool && result.gap <= puzzle->size) {
        if(result.gap < smallest_available_tile)
            is_solvable = false;
    }

    return is_solvable;
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
        random_tile = rand() % max_tile_size + 1;
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

int n_available_tiles(bool* valid_tiles) {
    int available_tiles = 0;
    for(int i = 0; i <= my_puzzle->size; ++i) {
        available_tiles += valid_tiles[i] ? 1 : 0;
    }
    return available_tiles;
}

void printNode(tree_node* ptr_node, FILE* file_ptr) {
    node_placement placement_data = *(node_placement*)ptr_node->data;
    bool* tile_bools = (bool*)&placement_data.valid_tiles;
    fprintf(file_ptr, "[Tile: %d - Pos.:(%2d,%2d) ", placement_data.tile_type,
            placement_data.x_pos, placement_data.y_pos);
    fprintf(file_ptr, "Valid Tiles: ");
    for(int i = 0; i < my_puzzle->size; ++i) {
        fprintf(file_ptr, "%d", tile_bools[i + 1] ? 1 : 0);
    }
    fprintf(file_ptr, "]\n");
}

// adapted from:
// https://www.geeksforgeeks.org/dsa/print-n-ary-tree-graphically/
void printTree(tree_node* ptr_node,
               int depth,
               bool isLast,
               bool* flag,
               FILE* file_ptr) {
    if(ptr_node == NULL) {
        fprintf(file_ptr, "[]\n");
        return;
    }

    // Loop to print the depths of the
    // current node
    for(int i = 1; i < depth; ++i) {
        // Condition when the depth
        // is exploring
        if(flag[i] == true) {
            fprintf(file_ptr, "│   ");
        }
        // Otherwise print
        // the blank spaces
        else {
            fprintf(file_ptr, "    ");
        }
    }

    // Condition when the current
    // node is the root node
    if(depth == 0) {
        printNode(ptr_node, file_ptr);
    } else if(isLast) {
        // Condition when the node is
        // the last node of
        // the exploring depth
        fprintf(file_ptr, "└───");
        printNode(ptr_node, file_ptr);
        // No more childrens turn it
        // to the non-exploring depth
        flag[depth] = false;
    } else {
        fprintf(file_ptr, "├───");
        printNode(ptr_node, file_ptr);
    }

    tree_node** pointer_to_children_pointers =
        (tree_node**)ptr_node->children.ptr_first_elem;
    tree_node* child_node_ptr = *pointer_to_children_pointers;

    for(size_t i = 0; i != ptr_node->children.dynarr_size; ++i) {
        // Recursive call for the
        // children nodes
        bool isLastChild = i == ptr_node->children.dynarr_size - 1;
        printTree(child_node_ptr, depth + 1, isLastChild, flag, file_ptr);

        child_node_ptr = *(++pointer_to_children_pointers);
    }
    flag[depth] = true;
}
