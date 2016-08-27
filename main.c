#include <stdio.h>
#include <time.h>

#include <allegro5/allegro.h>

#include "minesweeper.h"

int main(int argc, char **args) {
    srand(time(NULL));

    // Initialise allegro related things
    if (!al_init()) {
        fprintf(stderr, "Failed to initialise allegro\n");
        return -1;
    }
    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Failed to initialise primitives addon\n");
        return -1;
    }

    // Create the display
    ALLEGRO_DISPLAY *display = NULL;
    display = al_create_display(500, 500);
    if (!display) {
        fprintf(stderr, "Failed to create display\n");
        return -1;
    }

    // Create and register the event queue
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));

    // Game settings are hardcoded for now...
    int width = 4;
    int height = 4;
    int mine_count = 2;

    struct Game game;
    if (!init_game(&game, width, height, mine_count)) {
        fprintf(stderr, "Error initialising game\n");
        return -1;
    }
    print_grid(&(game.grid));

    int running = 1;
    while (running) {
        int x, y;
        read_coordinates(&(game.grid), &x, &y);
        reveal_cell(&game, x, y);
        print_grid(&(game.grid));

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