#pragma once

#include <elhaylib.h>
#include <puz.h>

typedef enum GAP_TYPE { VERTICAL, HORIZONTAL, NOT_FOUND } GAP_TYPE;
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
typedef void (*VIS_SET_F_PTR)(int, int, int);
typedef void (*VIS_SET_C_PTR)(int*, int);

void setup(int puzzle_type);
void set_visualizer(VIS_F_PTR grid_prep_func_in,
                    VIS_F_PTR grid_render_func_in,
                    VIS_F_PTR grid_reset_func_in,
                    VIS_F_PTR grid_record_func_in,
                    VIS_SET_F_PTR block_set_func_in,
                    VIS_SET_F_PTR block_remove_func_in,
                    VIS_SET_C_PTR block_set_color_func_in);
bool solution_search();
