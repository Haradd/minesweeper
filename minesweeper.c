#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minesweeper.h"

#define COLUMN_SEPERATOR '|'
#define ROW_SEPERATOR    '='
#define JOIN_SEPERATOR   '+'

#define MAX_WIDTH  26
#define MAX_HEIGHT 99

/*
 * Return the width of the decimal representation of the integer n
 */
int int_width(int n) {
    if (n == 0) {
        return 1;
    }

    int width = 0;
    while (n != 0) {
        n /= 10;
        width++;
    }
    return width;
}

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
int init_game(struct Game *game, int width, int height, int mine_count) {
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

    return 1;
}

/*
 * Print the provided grid to stdout
 */
void print_grid(struct Grid *grid) {

    // The width of the column to show numbers on the vertical axis
    int labels_width = int_width(grid->height + 1);

    // Print horizontal axis labels
    for (int i=0; i<labels_width + 3; i++) {
        // Print some spaces to offset the labels by the width of the vertical
        // axis labels. The + 3 is to account for the padding, seperator and
        // padding
        printf(" ");
    }
    for (int i=0; i<grid->width; i++) {
        // Get char code by adding 65
        printf("%c", i + 65);

        // Print the seperator if this is not the final column
        if (i + 1 < grid->width) {
            printf(" %c ", COLUMN_SEPERATOR);
        }
    }
    printf("\n");

    // Print a line seperating the horizontal axis labels from the actual data
    for (int i=0; i<labels_width + 1; i++) {
        // Print some spaces to offset by labels column width, + 1 for padding
        printf(" ");
    }
    printf("%c%c", JOIN_SEPERATOR, ROW_SEPERATOR);

    for (int i=0; i<grid->width; i++) {
        printf("%c", ROW_SEPERATOR);

        // Print the seperator if this is not the final column
        if (i + 1 < grid->width) {
            printf("%c%c%c", ROW_SEPERATOR, JOIN_SEPERATOR, ROW_SEPERATOR);
        }
    }
    printf("\n");

    // Print the actual grid
    for (int y=0; y<grid->height; y++) {

        // Print vertical axis label
        int padding = labels_width - int_width(y + 1);
        for (int i=0; i<padding; i++) {
            // Print some padding since if height >= 10 the columns will not
            // align properly otherwise
            printf(" ");
        }
        printf("%d %c ", y + 1, COLUMN_SEPERATOR);

        // Print row of data
        for (int x=0; x<grid->width; x++) {
            printf("%d", get_cell(grid, x, y));

            // Print the seperator if this is not the final column
            if (x + 1 < grid->width) {
                printf(" %c ", COLUMN_SEPERATOR);
            }
        }
        printf("\n");
    }
}

/*
 * Reveal a cell. If the cell contains a mine, show a message and exit the
 * program. If there are any adjacent mines, show the number and return.
 * If there are no adjacent mines, recursively reveal all adjacent cells that
 * have not already been revealed
 */
void reveal_cell(struct Game *game, int x, int y) {
    if (is_mine(game, x, y)) {
        show_mines(game);
        print_grid(&(game->grid));
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

/*
 * Read a pair of coordinates from the user (e.g. A5) and store them at the
 * location pointed to by x and y (e.g. x=0, y=4)
 *
 * @TODO: Fix bug: clear stdin on error, but in a way that means you don't have
 * enter a new line if you didn't type > 3 chars
 */
void read_coordinates(struct Grid *grid, int *x_ptr, int *y_ptr) {
    while (1) {
        printf("Enter coords: ");

        char input_string[4];
        fgets(input_string, 4, stdin);

        // The first character should be a letter describing the x-coordinate
        int x;
        if (input_string[0] >= 'A' && input_string[0] <= 'Z') {
            x = input_string[0] - 'A';
        }
        else if (input_string[0] >= 'a' && input_string[0] <= 'z') {
            x = input_string[0] - 'a';
        }
        else {
            // The character was not alphabetic
            printf("Invalid character '%c'\n", input_string[0]);
            continue;
        }

        char y_string[3];
        memccpy(y_string, input_string + 1, '\0', 3);

        int y;
        if (sscanf(y_string, "%d", &y) != 1) {
            printf("Invalid coordinate '%s'\n", y_string);
            continue;
        }
        y--;  // Decrease by 1 since we display the first row as y=1 to the user

        if (!valid_coords(grid, x, y)) {
            printf("Coordinates out of range\n");
            continue;
        }

        // If we have reached here then the coordinates must be valid
        *x_ptr = x;
        *y_ptr = y;
        break;
    }
}
