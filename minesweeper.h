#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#define CELL_TYPE_UNKNOWN -1
#define CELL_TYPE_MINE -2
#define CELL_TYPE_NO_MINES -3

struct Grid {
    int width;
    int height;
    int *cells;
};

struct Game {
    struct Grid grid;
    int mine_count;
    int *mines;
    int cells_revealed;
    int mine_exploded;

    int cell_size;  // The width/height of each cell in px
    int x_padding;  // The x offset of the grid in px
    int y_padding;  // The y offset of the grid in px
};

int init_game(struct Game *game, int width, int height, int mine_count,
              int display_width, int display_height);
void read_coordinates(struct Grid *grid, int *x, int *y);
void reveal_cell(struct Game *game, int x, int y);
int won_game(struct Game *game);
int get_cell(struct Grid *grid, int x, int y);
int valid_coords(struct Grid *grid, int x, int y);

#endif