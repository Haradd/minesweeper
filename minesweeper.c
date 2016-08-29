#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minesweeper.h"

#define MAX_WIDTH  99
#define MAX_HEIGHT 99

/*
 * Check that the provided coordinates are in range. Return 1 if they are,
 * 0 otherwise
 */
int valid_coords(struct Grid *grid, int x, int y) {
     if (x >= 0 && x < grid->width && y >= 0 && y < grid->height) {
        return 1;
     }
     else {
        return 0;
     }
}

/*
 * Return the value of the cell x, y for the specified grid
 */
int get_cell(struct Grid *grid, int x, int y) {
    if (valid_coords(grid, x, y)) {
        return grid->cells[x + y * grid->width];
    }
}

/*
 * Set the value of a cell in the grid. Return 1 if set succesfully, 0 otherwise
 */
int set_cell(struct Grid *grid, int x, int y, int value) {
    if (valid_coords(grid, x, y)) {
        grid->cells[x + y * grid->width] = value;
        return 1;
    }
    else {
        return 0;
    }
}

/*
 * Set the locations of the mines in the grid
 */
void show_mines(struct Game *game) {
    for (int i=0; i<game->mine_count; i++) {
        int x = game->mines[i] % game->grid.width;
        int y = game->mines[i] / game->grid.width;
        set_cell(&(game->grid), x, y, CELL_TYPE_MINE);
    }
}

/*
 * Return 1 if there is a mine at the specified coordinates, 0 otherwise
 */
int is_mine(struct Game *game, int x, int y) {
    int position = x + y * game->grid.width;
    for (int i=0; i<game->mine_count; i++) {
        if (game->mines[i] == position) {
            return 1;
        }
    }

    return 0;
}

/*
 * Return the number of mines adjacent to the specified location
 */
int adjacent_mines(struct Game *game, int x, int y) {
    int count = 0;
    for (int dx=-1; dx<=1; dx++) {
        for (int dy=-1; dy<=1; dy++) {

            // Skip this cell if (dx, dy) == (x, y)
            if (dx == 0 && dy == 0) {
                continue;
            }

            // Skip if this cell is not in the grid
            if (!valid_coords(&(game->grid), x + dx, y + dy)) {
                continue;
            }

            if (is_mine(game, x + dx, y + dy)) {
                count++;
            }
        }
    }

    return count;
}

/*
 * Initialise the grid, setting the value of each cell to seed. Return 1 if
 * successful, 0 otherwise
 */
int init_grid(struct Grid *grid, int width, int height, int seed) {
    if (width < 1 || width > MAX_WIDTH || height < 1 || height > MAX_HEIGHT) {
        printf("Invalid grid dimensions\n");
        return 0;
    }

    grid->width = width;
    grid->height = height;

    grid->cells = malloc(sizeof(int) * grid->width * grid->height);
    for (int y=0; y<grid->height; y++) {
        for (int x=0; x<grid->width; x++) {
            set_cell(grid, x, y, seed);
        }
    }

    return 1;
}

/*
 * Initialise the minesweeper game and place mines. Return 1 if succesful, 0
 * otherwise
 */
int init_game(struct Game *game, int width, int height, int mine_count,
              int display_width, int display_height) {

    if (!init_grid(&(game->grid), width, height, CELL_TYPE_UNKNOWN)) {
        return 0;
    }

    game->mine_count = mine_count;
    game->mines = malloc(sizeof(int) * mine_count);
    game->cells_revealed = 0;
    game->mine_exploded = 0;

    // Position the mines
    int mines_placed = 0;
    while (mines_placed < mine_count) {
        int r = rand() % (width * height);

        // Check that we have not already placed a mine in this position
        int collision = 0;
        for (int i=0; i<mines_placed; i++) {
            if (r == game->mines[i]) {
                collision = 1;
            }
        }

        if (!collision) {
            game->mines[mines_placed] = r;
            mines_placed++;
        }
    }

    // Calculate cell width in px and grid offset
    int x = display_width / game->grid.width;
    int y = display_height / game->grid.height;
    game->cell_size = (x < y ? x : y);

    game->x_padding = (display_width - game->cell_size * game->grid.width) / 2;
    game->y_padding = (display_height - game->cell_size * game->grid.height) / 2;

    return 1;
}

/*
 * Reveal a cell. If the cell contains a mine, set the mine_exploded flag and
 * return. If there are any adjacent mines, set the cell to the number and
 * return. If there are no adjacent mines, recursively reveal all adjacent cells
 * that have not already been revealed
 */
void reveal_cell(struct Game *game, int x, int y) {
    if (is_mine(game, x, y)) {
        show_mines(game);
        game->mine_exploded = 1;
    }
    else {
        game->cells_revealed++;
        int n = adjacent_mines(game, x, y);
        int cell_value;

        // Set the cell to 'no mines' if there are no adjacent mines, otherwise
        // set it to the number of adjacent mines
        if (n == 0) {
            cell_value = CELL_TYPE_NO_MINES;
        }
        else {
            cell_value = n;
        }

        set_cell(&(game->grid), x, y, cell_value);

        if (n == 0) {
            for (int dx=-1; dx<=1; dx++) {
                for (int dy=-1; dy<=1; dy++) {

                    int newX = x + dx;
                    int newY = y + dy;

                    // Skip if (x + dx, y + dy) is not in the grid
                    if (!valid_coords(&(game->grid), newX, newY)) {
                        continue;
                    }

                    // Skip this cell if it has already been revealed
                    if (get_cell(&(game->grid), newX, newY) != CELL_TYPE_UNKNOWN) {
                        continue;
                    }

                    reveal_cell(game, newX, newY);
                }
            }
        }
    }
}

/*
 * Set the specified cell to a flag if it is currently unknown, and set it to
 * unknown if it is currently a flag
 */
void toggle_flag(struct Grid *grid, int x, int y) {
    int cell_value = get_cell(grid, x, y);

    if (cell_value == CELL_TYPE_UNKNOWN) {
        set_cell(grid, x, y, CELL_TYPE_FLAG);
    }

    else if (cell_value == CELL_TYPE_FLAG) {
        set_cell(grid, x, y, CELL_TYPE_UNKNOWN);
    }
}

/*
 * Return 1 if the game has been won (i.e. all the remaining unrevealed cells
 * contain a mine), or 0 otherwise
 */
int won_game(struct Game *game) {
    int n = game->grid.width * game->grid.height - game->mine_count;
    if (game->cells_revealed == n) {
        return 1;
    }
    else {
        return 0;
    }
}
