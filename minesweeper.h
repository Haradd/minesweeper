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
};

int init_game(struct Game *game, int width, int height, int mine_count);
void read_coordinates(struct Grid *grid, int *x, int *y);
void reveal_cell(struct Game *game, int x, int y);
int won_game(struct Game *game);
void print_grid(struct Grid *grid);
int get_cell(struct Grid *grid, int x, int y);

#endif