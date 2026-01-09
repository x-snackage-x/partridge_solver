#define TESTING

typedef enum {
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    MAGENTA = 5,
    CYAN = 6,
    WHITE = 7,
    ORANGE = 202,
    ROYAL_BLUE = 17,
    GRAY = 240
} COLOR;

void prep_vis_grid(int size);
void render_vis_grid(int size);
void reset_vis_grid(int size);
void set_vis_block(int size, COLOR color, int x_pos, int y_pos);
