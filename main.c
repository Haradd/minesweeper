#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COLUMN_SEPERATOR '|'
#define ROW_SEPERATOR    '='
#define JOIN_SEPERATOR   '+'

#define MAX_WIDTH  26
#define MAX_HEIGHT 99

#define UNKOWN '-'
#define MINE   'M'

struct Grid {
    int width;
    int height;
    char *cells;
};

struct Game {
    struct Grid grid;
    int mine_count;
    int *mines;
};

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
            printf("%c", get_cell(grid, x, y));

            // Print the seperator if this is not the final column
            if (x + 1 < grid->width) {
                printf(" %c ", COLUMN_SEPERATOR);
            }
        }
        printf("\n");
    }
}

/*
 * Set the value of a cell in the grid. Return 1 if set succesfully, 0 otherwise
 */
int set_cell(struct Grid *grid, int x, int y, char value) {
    if (valid_coords(grid, x, y)) {
        grid->cells[x + y * grid->width] = value;
        return 1;
    }
    else {
        return 0;
    }
}

/*
 * Initialise the grid, setting the value of each cell to seed
 */
void init_grid(struct Grid *grid, int width, int height, char seed) {
    if (width < 1 || width > MAX_WIDTH || height < 1 || height > MAX_HEIGHT) {
        printf("Invalid grid dimensions\n");
        exit(1);
    }

    grid->width = width;
    grid->height = height;

    grid->cells = malloc(sizeof(int) * grid->width * grid->height);
    for (int y=0; y<grid->height; y++) {
        for (int x=0; x<grid->width; x++) {
            set_cell(grid, x, y, seed);
        }
    }
}

void init_game(struct Game *game, int width, int height, int mine_count) {
    init_grid(&(game->grid), width, height, UNKOWN);
    game->mine_count = mine_count;
    game->mines = malloc(sizeof(int) * mine_count);

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

            // Show the mine in the grid for debugging purposes
            int x = r % width;
            int y = r / width;
            set_cell(&(game->grid), x, y, MINE);

            mines_placed++;
        }
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

int main(int argc, char **args) {
    srand(time(NULL));

    // Game settings are hardcoded for now...
    int width = 8;
    int height = 8;
    int mine_count = 10;

    struct Game game;
    init_game(&game, width, height, mine_count);
    print_grid(&(game.grid));

    int running = 1;
    while (running) {
        int x, y;
        read_coordinates(&(game.grid), &x, &y);

        set_cell(&(game.grid), x, y, '1');
        print_grid(&(game.grid));
    }

    return 0;
}