#include <stdio.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "minesweeper.h"
#include "graphics.h"

#define DISPLAY_WIDTH 700
#define DISPLAY_HEIGHT 500

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;

// Colours
ALLEGRO_COLOR line_colour;
ALLEGRO_COLOR mine_colour;
ALLEGRO_COLOR unknown_colour;
ALLEGRO_COLOR no_mines_colour;
ALLEGRO_COLOR nearby_mines_colour[8];

ALLEGRO_FONT *font;

// These values are global since they are required in draw_grid() and draw_cell()
int cell_size;
int x_padding;
int y_padding;

/*
 * Initialise allegro and any allegro addons, and create a display and event
 * queue
 */
int init_allegro() {
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
    display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
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
 * Draw the actual minesweeper grid to the screen
 */
void draw_grid(struct Grid *grid) {

    int x = DISPLAY_WIDTH / grid->width;
    int y = DISPLAY_HEIGHT / grid->height;
    cell_size = (x < y ? x : y);

    x_padding = (DISPLAY_WIDTH - cell_size * grid->width) / 2;
    y_padding = (DISPLAY_HEIGHT - cell_size * grid->height) / 2;

    font = al_load_ttf_font("DejaVuSans.ttf", cell_size, 0);

    for (int i=0; i<grid->width; i++) {
        for (int j=0; j<grid->height; j++) {
            draw_cell(grid, i, j);
        }
    }

    // Draw grid lines
    for (int x=x_padding; x<=DISPLAY_WIDTH - x_padding; x+=cell_size) {
        al_draw_line(x, 0, x, DISPLAY_HEIGHT, line_colour, 3);
    }
    for (int y=y_padding; y<=DISPLAY_HEIGHT - y_padding; y+=cell_size) {
        al_draw_line(0, y, DISPLAY_WIDTH, y, line_colour, 3);
    }

    al_flip_display();
}

/*
 * Draw an individual cell to the screen
 */
void draw_cell(struct Grid *grid, int x, int y) {
    int value = get_cell(grid, x, y);

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

    int sx = x_padding + cell_size * x;
    int sy = y_padding + cell_size * y;
    al_draw_filled_rectangle(sx, sy, sx + cell_size, sy + cell_size, c);

    if (draw_text) {
        char text[] = {value + '0'};
        al_draw_text(font, al_map_rgb(0, 0, 0), sx, sy, 0, text);
    }
}
