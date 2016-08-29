#include <stdio.h>
#include <time.h>

#include <allegro5/allegro.h>

#include "minesweeper.h"
#include "graphics.h"

#define DISPLAY_WIDTH 700
#define DISPLAY_HEIGHT 500

/*
 * Work out if the clicked point is inside a cell, and put the cell coordinates
 * in the addresses pointed to by x_ptr and y_ptr. Return 1 if a cell was
 * clicked, 0 otherwise
 */
int get_clicked_cell(struct Game *game, int mouse_x, int mouse_y, int *x_ptr,
                     int *y_ptr) {

    int x_offset = mouse_x - game->x_padding;
    int y_offset = mouse_y - game->y_padding;

    // If the click was within the grid area...
    if (x_offset >= 0 && x_offset <= game->grid.width * game->cell_size &&
        y_offset >= 0 && y_offset <= game->grid.height * game->cell_size) {

        *x_ptr = x_offset / game->cell_size;
        *y_ptr = y_offset / game->cell_size;

        return 1;
    }
    else {
        return 0;
    }
}

int main(int argc, char **args) {

    srand(time(NULL));

    ALLEGRO_DISPLAY *display;
    ALLEGRO_EVENT_QUEUE *event_queue;
    if (!init_allegro(DISPLAY_WIDTH, DISPLAY_HEIGHT, &display, &event_queue)) {
        return -1;
    }

    // Game settings are hardcoded for now...
    int width = 8;
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

        ALLEGRO_EVENT event;
        ALLEGRO_TIMEOUT timeout;
        al_init_timeout(&timeout, 0.06);

        // Wait for an event to be received or for timeout
        bool event_received = al_wait_for_event_until(event_queue, &event,
                                                      &timeout);

        if (event_received) {
            // Close window if close button was pressed
            if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                return 0;
            }

            // Handle mouse clicks
            else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
                int x, y;
                if (get_clicked_cell(&game, event.mouse.x, event.mouse.y,
                                     &x, &y)) {

                    // Left click - reveal the cell
                    if (event.mouse.button == 1) {
                        reveal_cell(&game, x, y);
                    }
                    // Right click - place flag
                    else if (event.mouse.button == 2) {
                        printf("Placing flag\n");
                    }

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
            }
        }
    }

    return 0;
}