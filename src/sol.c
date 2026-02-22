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
bool line_scan_hor(puzzle_def* puzzle, point* result);
bool find_smallest_gap(puzzle_def* puzzle, gap_search_result* res_struct);

void printNode(tree_node* ptr_node);
void printTree(tree_node* ptr_node, int depth, bool isLast, bool* flag);

void setup(int puzzle_type) {
    my_puzzle = calloc(1, sizeof(puzzle_def));

    my_puzzle->size = puzzle_type;
    init_puzzle(my_puzzle);

    node_size = sizeof(node_placement);
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
    memset(valid_tiles, true, sizeof(bool) * (puzzle_type + 1));
    valid_tiles[0] = false;

    point result_buffer = {0};
    place_block(my_puzzle, selected_tile, result_buffer.x_index,
                result_buffer.y_index);
    printf("Placed Root tile: %d\n", selected_tile);

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

    srand(time(NULL));
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

    return tree_result.node_ptr;
}

void solution_search() {
    bool* flags = malloc(sizeof(bool) * placement_record.tree_size);
    int puzzle_type = my_puzzle->size;

    is_solvable = is_solvable_first_check(my_puzzle);
    is_solved = is_puzzle_solved(my_puzzle);

    bool* valid_tiles;

    point result_buffer;
    int loop_n = 0;
    while(!is_solved && loop_n++ < 100) {
        line_scan_hor(my_puzzle, &result_buffer);

        node_placement* placement_data = (node_placement*)last_placement->data;
        valid_tiles = (bool*)&placement_data->valid_tiles;

        // select one tile and place
        bool is_placeable;
        bool tiles_available = true;
        do {
            int selected_tile = random_tile_select(valid_tiles, puzzle_type);
            valid_tiles[selected_tile] = false;
            // printf("Current tile: %d", selected_tile);
            is_placeable = placement_resolvable(my_puzzle, selected_tile,
                                                result_buffer.x_index,
                                                result_buffer.y_index);
            RETURN_CODES placement_code =
                place_block(my_puzzle, selected_tile, result_buffer.x_index,
                            result_buffer.y_index);
            is_placeable &= placement_code == SUCCESS;
            if(is_placeable) {
                // printf(" - Placement success: true\n");

                last_placement =
                    record_placement(selected_tile, result_buffer.x_index,
                                     result_buffer.y_index, last_placement);
            } else {
                // printf(" - Placement success: false\n");
            }
            memset(flags, true, placement_record.tree_size);
            printf(
                "--------------------------------------------------------------"
                "------------------------------------------------------------"
                "\n");
            printTree(placement_record.tree_root, 0, false, flags);

            int available_tiles = 0;
            for(int i = 0; i <= my_puzzle->size; ++i) {
                available_tiles += valid_tiles[i] ? 1 : 0;
            }
            tiles_available = available_tiles > 0;
        } while(!is_placeable && tiles_available);

        is_solvable = is_solvable_first_check(my_puzzle);
        is_solvable &= tiles_available;
        if(!is_solvable) {
            tree_node* parent = last_placement->parent;
            node_placement placement_data =
                *(node_placement*)last_placement->data;
            remove_block(my_puzzle, placement_data.tile_type,
                         placement_data.x_pos, placement_data.y_pos);
            printf("Remove tile: %d\n", placement_data.tile_type);
            last_placement = parent;
        }

        if(!tiles_available) {
            tree_node* parent = last_placement->parent;
            node_placement placement_data =
                *(node_placement*)last_placement->data;
            remove_block(my_puzzle, placement_data.tile_type,
                         placement_data.x_pos, placement_data.y_pos);
            printf("Remove tile: %d\n", placement_data.tile_type);
            last_placement = parent;
        }

        is_solved = is_puzzle_solved(my_puzzle);
    }
}

int main() {
    int puzzle_type = 8;
    setup(puzzle_type);
    solution_search();

    printf("Puzzle Status: Solvable: %s - Solved: %s\n",
           is_solvable ? "true" : "false", is_solved ? "true" : "false");
    print_grid(my_puzzle);

    bool* flags = malloc(sizeof(bool) * placement_record.tree_size);
    memset(flags, true, placement_record.tree_size);

    printTree(placement_record.tree_root, 0, false, flags);
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
        random_tile = rand() % (max_tile_size + 1);
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

void printNode(tree_node* ptr_node) {
    node_placement placement_data = *(node_placement*)ptr_node->data;
    bool* tile_bools = (bool*)&placement_data.valid_tiles;
    printf("[Tile: %d - Pos.:(%2d,%2d) ", placement_data.tile_type,
           placement_data.x_pos, placement_data.y_pos);
    printf("Valid Tiles: ");
    for(int i = 0; i < my_puzzle->size; ++i) {
        printf("%d", tile_bools[i + 1] ? 1 : 0);
    }
    printf("]\n");
}

// adapted from: https://www.geeksforgeeks.org/dsa/print-n-ary-tree-graphically/
void printTree(tree_node* ptr_node, int depth, bool isLast, bool* flag) {
    if(ptr_node == NULL) {
        printf("[]\n");
        return;
    }

    // Loop to print the depths of the
    // current node
    for(int i = 1; i < depth; ++i) {
        // Condition when the depth
        // is exploring
        if(flag[i] == true) {
            printf("│   ");
        }
        // Otherwise print
        // the blank spaces
        else {
            printf("    ");
        }
    }

    // Condition when the current
    // node is the root node
    if(depth == 0) {
        printNode(ptr_node);
    } else if(isLast) {
        // Condition when the node is
        // the last node of
        // the exploring depth
        printf("└───");
        printNode(ptr_node);
        // No more childrens turn it
        // to the non-exploring depth
        flag[depth] = false;
    } else {
        printf("├───");
        printNode(ptr_node);
    }

    tree_node** pointer_to_children_pointers =
        (tree_node**)ptr_node->children.ptr_first_elem;
    tree_node* child_node_ptr = *pointer_to_children_pointers;

    for(size_t i = 0; i != ptr_node->children.dynarr_size; ++i) {
        // Recursive call for the
        // children nodes
        bool isLastChild = i == ptr_node->children.dynarr_size - 1;
        printTree(child_node_ptr, depth + 1, isLastChild, flag);

        child_node_ptr = *(++pointer_to_children_pointers);
    }
    flag[depth] = true;
}
