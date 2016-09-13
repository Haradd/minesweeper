#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#define CELL_TYPE_UNKNOWN -1
#define CELL_TYPE_MINE -2
#define CELL_TYPE_NO_MINES -3
#define CELL_TYPE_FLAG -4

struct Game {
    int width;
    int height;
    int *cells;
    int mine_count;
    int *mines;
    int cells_revealed;
    int mine_exploded;

    // Number of flags remaning. Note that this can be negative if the user has
    // placed more flags than there are mines
    int flags_remaining;

    float cell_size;  // The width/height of each cell in px
    float x_padding;  // The x offset of the grid in px
    float y_padding;  // The y offset of the grid in px

    // Timestamp of when the game started
    time_t timestamp;
};

int init_game(struct Game *game, int width, int height, int mine_count,
              int display_width, int display_height, int padding);
void reveal_neighobouring_cells(struct Game *game, int x, int y);
void reveal_cell(struct Game *game, int x, int y);
int won_game(struct Game *game);
int lost_game(struct Game *game);
int get_cell(struct Game *game, int x, int y);
void toggle_flag(struct Game *game, int x, int y);

#endif