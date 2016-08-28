#include <stdio.h>
#include <time.h>

#include <allegro5/allegro.h>

#include "minesweeper.h"
#include "graphics.h"

#define DISPLAY_WIDTH 700
#define DISPLAY_HEIGHT 500

int main(int argc, char **args) {
    srand(time(NULL));

    init_allegro(DISPLAY_WIDTH, DISPLAY_HEIGHT);

    // Game settings are hardcoded for now...
    int width = 5;
    int height = 8;
    int mine_count = 10;

    struct Game game;
    if (!init_game(&game, width, height, mine_count, DISPLAY_WIDTH,
                   DISPLAY_HEIGHT)) {

        fprintf(stderr, "Error initialising game\n");
        return -1;
    }

    draw_grid(&game);

    int running = 1;
    while (running) {
        int x, y;
        read_coordinates(&(game.grid), &x, &y);
        reveal_cell(&game, x, y);
        draw_grid(&game);

        if (game.mine_exploded) {
            printf("You hit a mine!\n");
            return 0;
        }

        if (won_game(&game)) {
            printf("You won!\n");
            return 0;
        }
    }

    return 0;
}