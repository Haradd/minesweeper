#include <stdio.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "minesweeper.h"
#include "graphics.h"

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;

// Colours
ALLEGRO_COLOR line_colour;
ALLEGRO_COLOR mine_colour;
ALLEGRO_COLOR unknown_colour;
ALLEGRO_COLOR no_mines_colour;
ALLEGRO_COLOR nearby_mines_colour[8];

ALLEGRO_FONT *font;

/*
 * Initialise allegro and any allegro addons, and create a display and event
 * queue
 */
int init_allegro(int width, int height) {
    if (!al_init()) {
        fprintf(stderr, "Failed to initialise allegro\n");
        return 0;
    }
    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Failed to initialise primitives addon\n");
        return 0;
    }

    al_init_font_addon();
    if (!al_init_ttf_addon()) {
        fprintf(stderr, "Failed to initialise TTF font addon\n");
        return 0;
    }

    // Create the display
    display = NULL;
    display = al_create_display(width, height);
    if (!display) {
        fprintf(stderr, "Failed to create display\n");
        return 0;
    }

    // Create and register the event queue
    event_queue = NULL;
    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));

    // Create the colours
    line_colour = al_map_rgb(10, 10, 10);
    mine_colour = al_map_rgb(255, 0, 0);
    unknown_colour = al_map_rgb(180, 180, 180);
    no_mines_colour = al_map_rgb(240, 240, 240);
    nearby_mines_colour[0] = al_map_rgb(0, 0, 200);
    nearby_mines_colour[1] = al_map_rgb(0, 200, 0);
    nearby_mines_colour[2] = al_map_rgb(200, 0, 0);
    nearby_mines_colour[3] = al_map_rgb(200, 200, 0);
    nearby_mines_colour[4] = al_map_rgb(200, 0, 200);
    nearby_mines_colour[5] = al_map_rgb(0, 200, 200);
    nearby_mines_colour[6] = al_map_rgb(200, 200, 200);
    nearby_mines_colour[7] = al_map_rgb(200, 50, 200);

    return 1;
}

/*
 * Draw an individual cell to the screen
 */
void draw_cell(struct Game *game, int x, int y) {
    int value = get_cell(&(game->grid), x, y);

    int draw_text = 0;
    ALLEGRO_COLOR c;
    switch (value) {
        case CELL_TYPE_MINE:
            c = mine_colour;
            break;

        case CELL_TYPE_UNKNOWN:
            c = unknown_colour;
            break;

        case CELL_TYPE_NO_MINES:
            c = no_mines_colour;
            break;

        default:
            c = nearby_mines_colour[value - 1];
            draw_text = 1;
    }

    int sx = game->x_padding + game->cell_size * x;
    int sy = game->y_padding + game->cell_size * y;
    al_draw_filled_rectangle(sx, sy, sx + game->cell_size, sy + game->cell_size,
                             c);

    if (draw_text) {
        char text[] = {value + '0'};
        al_draw_text(font, al_map_rgb(0, 0, 0), sx, sy, 0, text);
    }
}

/*
 * Draw the actual minesweeper grid to the screen
 */
void draw_grid(struct Game *game) {

    font = al_load_ttf_font("DejaVuSans.ttf", game->cell_size, 0);

    // Draw the cells
    for (int i=0; i<game->grid.width; i++) {
        for (int j=0; j<game->grid.height; j++) {
            draw_cell(game, i, j);
        }
    }

    // Draw grid lines
    for (int i=0; i<game->grid.width; i++) {
        int x = game->x_padding + i * game->cell_size;
        al_draw_line(x, 0, x,
                     game->y_padding + game->grid.height * game->cell_size,
                     line_colour, 3);
    }

    for (int i=0; i<game->grid.height; i++) {
        int y = game->y_padding + i * game->cell_size;
        al_draw_line(0, y, game->x_padding + game->grid.width * game->cell_size,
                     y, line_colour, 3);
    }

    al_flip_display();
}