#pragma once

#include <elhaylib.h>
#include <puz.h>

typedef enum GAP_TYPE { VERTIICAL, HORIZONTAL, NOT_FOUND } GAP_TYPE;
typedef struct {
    int x_index;
    int y_index;
    int gap;
    GAP_TYPE type;
} gap_search_result;

typedef struct {
    int x_index;
    int y_index;
} point;

typedef void (*VIS_F_PTR)(int);
typedef void (*VIS_SET_F_PTR)(int, int, int, int);

void setup(int puzzle_type);
void set_visualizer(VIS_F_PTR grid_prep_func,
                    VIS_F_PTR grid_render_func,
                    VIS_F_PTR grid_reset_func,
                    VIS_SET_F_PTR block_set_func);

bool is_solvable_first_check(puzzle_def* puzzle);

bool line_scan_hor(puzzle_def* puzzle, point* result);
bool find_smallets_gap(puzzle_def* puzzle, gap_search_result* res_struct);