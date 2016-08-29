#include <stdio.h>
#include <string.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "minesweeper.h"
#include "graphics.h"

#define FONT_NAME "DejaVuSans.ttf"
#define TITLE_FONT_SIZE 20
#define BUTTON_FONT_SIZE 15

// The vertical space between each button
#define BUTTON_MARGIN 30
// The vertical space between the text and the sides of the button
#define BUTTON_PADDING 10

// Colours
ALLEGRO_COLOR line_colour;
ALLEGRO_COLOR mine_colour;
ALLEGRO_COLOR unknown_colour;
ALLEGRO_COLOR no_mines_colour;
ALLEGRO_COLOR nearby_mines_colour[8];
ALLEGRO_COLOR flag_colour;

ALLEGRO_FONT *cell_font;
ALLEGRO_FONT *title_font;
ALLEGRO_FONT *button_font;

/*
 * Initialise allegro and any allegro addons, and create a display and event
 * queue. Return 1 if successful, 0 otherwise
 */
int init_allegro(int width, int height, ALLEGRO_DISPLAY **display,
                 ALLEGRO_EVENT_QUEUE **event_queue) {

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
    title_font = al_load_ttf_font(FONT_NAME, TITLE_FONT_SIZE, 0);
    button_font = al_load_ttf_font(FONT_NAME, BUTTON_FONT_SIZE, 0);

    if (!al_install_mouse()) {
        fprintf(stderr, "Failed to install mouse\n");
        return 0;
    }

    // Create the display
    *display = al_create_display(width, height);
    if (!(*display)) {
        fprintf(stderr, "Failed to create display\n");
        return 0;
    }

    // Create and register the event queue
    *event_queue = al_create_event_queue();
    if (!(*event_queue)) {
        fprintf(stderr, "Failed to create event queue\n");
        return 0;
    }
    al_register_event_source(*event_queue, al_get_display_event_source(*display));
    al_register_event_source(*event_queue, al_get_mouse_event_source());

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
    flag_colour = al_map_rgb(255, 0, 100);

    return 1;
}

/*
 * Draw an individual cell to the screen
 */
void draw_cell(struct Game *game, int x, int y) {
    int value = get_cell(&(game->grid), x, y);

    char text = 0;
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

        case CELL_TYPE_FLAG:
            c = flag_colour;
            text = 'F';
            break;

        default:
            c = nearby_mines_colour[value - 1];
            text = value + '0';
    }

    int sx = game->x_padding + game->cell_size * x;
    int sy = game->y_padding + game->cell_size * y;
    al_draw_filled_rectangle(sx, sy, sx + game->cell_size, sy + game->cell_size,
                             c);

    if (text != 0) {
        char string[] = {text};
        al_draw_text(cell_font, al_map_rgb(0, 0, 0), sx, sy, 0, string);
    }
}

/*
 * Draw the actual minesweeper grid to the screen
 */
void draw_grid(struct Game *game) {

    cell_font = al_load_ttf_font(FONT_NAME, game->cell_size, 0);

    // Draw the cells
    for (int i=0; i<game->grid.width; i++) {
        for (int j=0; j<game->grid.height; j++) {
            draw_cell(game, i, j);
        }
    }

    // Draw grid lines
    for (int i=0; i<game->grid.width; i++) {
        int x = game->x_padding + i * game->cell_size;
        al_draw_line(x, game->y_padding, x,
                     game->y_padding + game->grid.height * game->cell_size,
                     line_colour, 3);
    }

    for (int i=0; i<game->grid.height; i++) {
        int y = game->y_padding + i * game->cell_size;
        al_draw_line(game->x_padding, y,
                     game->x_padding + game->grid.width * game->cell_size, y,
                     line_colour, 3);
    }

    al_flip_display();
}

/*
 * Set up the id, label and coordinates of a button
 */
void create_button(struct Button *button, int id, char *label, int x, int y) {
    button->id = id;
    strncpy(button->label, label, MAX_BUTTON_LENGTH);
    button->x = x;
    button->y = y;
}

/*
 * Calculate the coordinates of the corners of the rectangle for a given button.
 * The top left coordinates are stored in x1, y1, and bottom right in x2, y2
 */
void get_button_rect(struct Button *button, int *x1, int *y1, int *x2, int *y2) {
    int bbx, bby, width, height;
    al_get_text_dimensions(button_font, button->label, &bbx, &bby, &width,
                           &height);

    *x1 = button->x - 0.5 * width - BUTTON_PADDING;
    *y1 = button->y - 0.5 * height - BUTTON_PADDING;
    *x2 = *x1 + width + 2 * BUTTON_PADDING;
    *y2 = *y1 + height + 2 * BUTTON_PADDING;
}

/*
 * Draw the provided button to the screen
 */
void draw_button(struct Button *button) {
    int x1, x2, y1, y2;
    get_button_rect(button, &x1, &y1, &x2, &y2);

    al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(200, 200, 200));

    al_draw_text(button_font, al_map_rgb(0, 0, 0), x1 + BUTTON_PADDING,
                 y1 + BUTTON_PADDING, 0, button->label);

}

/*
 * Work out if a button in the array of buttons is at the specified coordinates.
 * Return the ID of the clicked button, or -1 if no button was found
 */
int get_clicked_button(struct Button *buttons, int count, int mouse_x,
                       int mouse_y) {

    for (int i=0; i<count; i++) {
        int x1, x2, y1, y2;
        get_button_rect(&(buttons[i]), &x1, &y1, &x2, &y2);

        if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1 && mouse_y <= y2) {
            return buttons[i].id;
        }
    }

    // No button was clicked - return -1;
    return -1;
}
