#ifndef MINESWEEPER_H
#define MINESWEEPER_H

struct Grid {
    int width;
    int height;
    char *cells;
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

#endif