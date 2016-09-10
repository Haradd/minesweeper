#include <stdio.h>
#include <string.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "minesweeper.h"
#include "graphics.h"

#define GRAPHICS_FPS 30

// TODO: Fix this
#define FONT_NAME "/home/joe/Coding/minesweeper/assets/DejaVuSans.ttf"
#define TITLE_FONT_SIZE 20
#define BUTTON_FONT_SIZE 30

// The vertical space between each button
#define BUTTON_MARGIN 30
// The vertical space between the text and the sides of the button
#define BUTTON_PADDING 30

// Colours
ALLEGRO_COLOR line_colour;
ALLEGRO_COLOR mine_colour;
ALLEGRO_COLOR unknown_colour;
ALLEGRO_COLOR no_mines_colour;
ALLEGRO_COLOR nearby_mines_colour[8];
ALLEGRO_COLOR flag_colour;
ALLEGRO_COLOR background_colour;
ALLEGRO_COLOR button_background_colour;
ALLEGRO_COLOR button_hover_colour;
ALLEGRO_COLOR button_text_colour;
ALLEGRO_COLOR label_colour;

ALLEGRO_FONT *cell_font;
ALLEGRO_FONT *title_font;
ALLEGRO_FONT *button_font;

/*
 * Initialise allegro and any allegro addons, and create a display and event
 * queue. Return 1 if successful, 0 otherwise
 */
int init_allegro(int width, int height, ALLEGRO_DISPLAY **display,
                 ALLEGRO_EVENT_QUEUE **event_queue, ALLEGRO_TIMER **timer) {

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

    // Create and start the timer
    *timer = al_create_timer(1.0 / GRAPHICS_FPS);
    if (!(*timer)) {
        fprintf(stderr, "Failed to create timer\n");
        return 0;
    }
    al_start_timer(*timer);

    // Create and register the event queue
    *event_queue = al_create_event_queue();
    if (!(*event_queue)) {
        fprintf(stderr, "Failed to create event queue\n");
        return 0;
    }
    al_register_event_source(*event_queue, al_get_display_event_source(*display));
    al_register_event_source(*event_queue, al_get_mouse_event_source());
    al_register_event_source(*event_queue, al_get_timer_event_source(*timer));

    // Create the colours
    line_colour =              al_map_rgb(10, 10, 10);
    mine_colour =              al_map_rgb(255, 0, 0);
    unknown_colour =           al_map_rgb(180, 180, 180);
    no_mines_colour =          al_map_rgb(240, 240, 240);
    nearby_mines_colour[0] =   al_map_rgb(0, 0, 200);
    nearby_mines_colour[1] =   al_map_rgb(0, 200, 0);
    nearby_mines_colour[2] =   al_map_rgb(200, 0, 0);
    nearby_mines_colour[3] =   al_map_rgb(200, 200, 0);
    nearby_mines_colour[4] =   al_map_rgb(200, 0, 200);
    nearby_mines_colour[5] =   al_map_rgb(0, 200, 200);
    nearby_mines_colour[6] =   al_map_rgb(200, 200, 200);
    nearby_mines_colour[7] =   al_map_rgb(200, 50, 200);
    flag_colour =              al_map_rgb(255, 0, 100);
    background_colour =        al_map_rgb(127, 127, 127);
    button_background_colour = al_map_rgb(200, 200, 200);
    button_hover_colour =      al_map_rgb(170, 170, 170);
    button_text_colour =       al_map_rgb(0, 0, 0);
    label_colour =             al_map_rgb(200, 200, 200);

    return 1;
}

/*
 * Draw an individual cell to the screen
 */
void draw_cell(struct Game *game, int x, int y, int hovered) {
    int value = get_cell(game, x, y);

    char text = 0;
    ALLEGRO_COLOR text_colour;
    ALLEGRO_COLOR cell_colour;
    switch (value) {
        case CELL_TYPE_MINE:
            cell_colour = mine_colour;
            break;

        case CELL_TYPE_UNKNOWN:
            cell_colour = unknown_colour;
            break;

        case CELL_TYPE_NO_MINES:
            cell_colour = no_mines_colour;
            break;

        case CELL_TYPE_FLAG:
            cell_colour = no_mines_colour;
            text_colour = flag_colour;
            text = 'F';
            break;

        default:
            cell_colour = no_mines_colour;
            text_colour = nearby_mines_colour[value - 1];
            text = value + '0';
    }

    int sx = game->x_padding + game->cell_size * x;
    int sy = game->y_padding + game->cell_size * y;

    // Draw cell background colour
    al_draw_filled_rectangle(sx, sy, sx + game->cell_size, sy + game->cell_size,
                             cell_colour);

    // Draw cell border
    float line_width = hovered ? 2 : 1;
    al_draw_rectangle(sx + line_width / 2, sy + line_width / 2,
                      sx + game->cell_size - line_width / 2,
                      sy + game->cell_size - line_width / 2,
                      line_colour, line_width);

    if (text != 0) {
        char string[] = {text};
        al_draw_text(cell_font, text_colour, sx, sy, 0, string);
    }
}

/*
 * Draw the actual minesweeper grid to the screen
 */
void draw_game(struct Game *game) {

    cell_font = al_load_ttf_font(FONT_NAME, game->cell_size, 0);

    draw_background();

    // Draw the cells
    for (int i=0; i<game->width; i++) {
        for (int j=0; j<game->height; j++) {
            draw_cell(game, i, j, 0);
        }
    }
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
void draw_button(struct Button *button, int hovered) {
    int x1, x2, y1, y2;
    get_button_rect(button, &x1, &y1, &x2, &y2);

    ALLEGRO_COLOR c = (hovered ? button_hover_colour : button_background_colour);
    al_draw_filled_rectangle(x1, y1, x2, y2, c);

    al_draw_text(button_font, button_text_colour, x1 + BUTTON_PADDING,
                 y1 + BUTTON_PADDING, 0, button->label);
}

/*
 * Draw the provided label
 */
void draw_label(struct Label *label) {
    // Load font and get text dimensions
    ALLEGRO_FONT *font = al_load_ttf_font(FONT_NAME, label->font_size, 0);
    int bbx, bby, width, height;
    al_get_text_dimensions(font, label->text, &bbx, &bby, &width, &height);

    int x = label->x - 0.5 * width;
    int y = label->y - 0.5 * height;

    printf("bbx, bby, width, height: %d, %d, %d, %d\n", bbx, bby ,width, height);
    printf("Lbel coords are: %d, %d\n", label->x, label->y);
    printf("Calculated coords are: %d, %d\n", x, y);

    al_draw_text(font, label_colour, x, y, 0, label->text);
}

/*
 * Work out if a button in the array of buttons pointers is at the specified
 * coordinates. Return a pointer to the clicked button, or NULL if no button was
 * found
 */
struct Button *get_clicked_button(struct Button **buttons, int count, int mouse_x,
                                  int mouse_y) {

    for (int i=0; i<count; i++) {
        int x1, x2, y1, y2;
        get_button_rect(buttons[i], &x1, &y1, &x2, &y2);

        if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1 && mouse_y <= y2) {
            return buttons[i];
        }
    }

    // No button was clicked
    return NULL;
}

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
    if (x_offset >= 0 && x_offset < game->width * game->cell_size &&
        y_offset >= 0 && y_offset < game->height * game->cell_size) {

        *x_ptr = x_offset / game->cell_size;
        *y_ptr = y_offset / game->cell_size;

        return 1;
    }
    else {
        return 0;
    }
}

/*
 * Draw a semi-transparent rectangle between the specified coordinates
 */
void shade_screen(int x1, int y1, int x2, int y2) {
    int r = 50;
    int g = 50;
    int b = 50;
    int a = 150;
    ALLEGRO_COLOR c = al_map_rgba(r * a / 255, g * a / 255, b * a / 255, a);
    al_draw_filled_rectangle(x1, y1,x2, y2, c);
}

/*
 * Fill the whole display with the background colour
 */
void draw_background(int width, int height) {
    al_clear_to_color(background_colour);
}
