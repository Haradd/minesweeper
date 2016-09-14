#include <stdio.h>
#include <string.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>

#include "minesweeper.h"
#include "graphics.h"

#define GRAPHICS_FPS 30

#define FONT_NAME "DejaVuSans.ttf"
#define TITLE_FONT_SIZE 20
#define BUTTON_FONT_SIZE 30

// The vertical space between the text and the sides of the button
#define BUTTON_PADDING 30

// The number of images used in the application
#define IMAGE_COUNT 2

// A struct to store and retreive ALLEGRO_BITMAPs by filename
struct BitmapContainer {
    int count;
    char names[IMAGE_COUNT][20];
    void *bitmaps[IMAGE_COUNT];
};

// Colours
ALLEGRO_COLOR line_colour;
ALLEGRO_COLOR mine_colour;
ALLEGRO_COLOR unknown_colour;
ALLEGRO_COLOR no_mines_colour;
ALLEGRO_COLOR nearby_mines_colour[8];
ALLEGRO_COLOR background_colour;
ALLEGRO_COLOR button_background_colour;
ALLEGRO_COLOR button_hover_colour;
ALLEGRO_COLOR button_text_colour;
ALLEGRO_COLOR label_colour;

ALLEGRO_FONT *cell_font;
ALLEGRO_FONT *title_font;
ALLEGRO_FONT *button_font;

// Paths to game assets
char asset_dir[200];
char font_path[200];

struct BitmapContainer bitmap_container;

/*
 * Initialise allegro and any allegro addons, and create a display and event
 * queue. Return 1 if successful, 0 otherwise
 */
int init_allegro(int width, int height, ALLEGRO_DISPLAY **display,
                 ALLEGRO_EVENT_QUEUE **event_queue, ALLEGRO_TIMER **timer,
                 char *asset_dir_p) {

    // Set paths to game assets
    strcpy(asset_dir, asset_dir_p);
    sprintf(font_path, "%s/%s", asset_dir, FONT_NAME);

    // Initialise the bitmap collection
    bitmap_container.count = 0;

    if (!al_init()) {
        fprintf(stderr, "Failed to initialise allegro\n");
        return 0;
    }
    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Failed to initialise primitives addon\n");
        return 0;
    }

    if (!al_init_image_addon()) {
        fprintf(stderr, "Failed to initialise image addon\n");
        return 0;
    }

    al_init_font_addon();
    if (!al_init_ttf_addon()) {
        fprintf(stderr, "Failed to initialise TTF font addon\n");
        return 0;
    }
    title_font = al_load_ttf_font(font_path, TITLE_FONT_SIZE, 0);
    button_font = al_load_ttf_font(font_path, BUTTON_FONT_SIZE, 0);

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
    background_colour =        al_map_rgb(127, 127, 127);
    button_background_colour = al_map_rgb(200, 200, 200);
    button_hover_colour =      al_map_rgb(170, 170, 170);
    button_text_colour =       al_map_rgb(0, 0, 0);
    label_colour =             al_map_rgb(200, 200, 200);

    return 1;
}

/*
 * Retreive a bitmap by filename from the bitmap collection, or load it and add
 * it to the collection if not present
 */
ALLEGRO_BITMAP *get_bitmap(char *name) {
    // Try to find the provided name in the names array in bitmap_collection
    for (int i=0; i<bitmap_container.count; i++) {
        if (strcmp(name, bitmap_container.names[i]) == 0) {
            return bitmap_container.bitmaps[i];
        }
    }
    // If reached here then the bitmap has not been found, so create it
    char path[200];
    sprintf(path, "%s/%s", asset_dir, name);
    ALLEGRO_BITMAP *bmp = al_load_bitmap(path);

    // Store the name and bitmap
    strcpy(bitmap_container.names[bitmap_container.count], name);
    bitmap_container.bitmaps[bitmap_container.count] = bmp;
    bitmap_container.count++;

    return bmp;
}

/*
 * Draw an individual cell to the screen
 */
void draw_cell(struct Game *game, int x, int y, int hovered) {
    int value = get_cell(game, x, y);

    char text = 0;
    ALLEGRO_COLOR text_colour;
    ALLEGRO_COLOR cell_colour;

    char image_name[60];
    image_name[0] = '\0';

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
            cell_colour = unknown_colour;
            strcpy(image_name, "flag.png");
            break;

        default:
            cell_colour = no_mines_colour;
            text_colour = nearby_mines_colour[value - 1];
            text = value + '0';
    }

    float sx = game->x_padding + game->cell_size * x;
    float sy = game->y_padding + game->cell_size * y;

    // Draw cell background colour
    al_draw_filled_rectangle(sx, sy, sx + game->cell_size, sy + game->cell_size,
                             cell_colour);

    // Draw cell border
    float line_width = hovered ? 2 : 1;
    al_draw_rectangle(sx + line_width / 2, sy + line_width / 2,
                      sx + game->cell_size - line_width / 2,
                      sy + game->cell_size - line_width / 2,
                      line_colour, line_width);

    // Draw text if text has been specified
    if (text != 0) {
        char string[] = {text};
        al_draw_text(cell_font, text_colour, sx + 0.5 * game->cell_size, sy,
                     ALLEGRO_ALIGN_CENTRE, string);
    }

    // Draw an image if image_name has been set
    if (strlen(image_name) > 0) {
        draw_image(image_name, sx, sy, game->cell_size, game->cell_size);
    }
}

/*
 * Draw the actual minesweeper grid to the screen
 */
void draw_game(struct Game *game) {

    cell_font = al_load_ttf_font(font_path, game->cell_size, 0);

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
 * Load a font for the specified font size and store it in the provided label
 */
void set_label_font(struct Label *label, int font_size) {
    label->font_size = font_size;
    label->font = al_load_ttf_font(font_path, font_size, 0);
}

/*
 * Draw the provided label
 */
void draw_label(struct Label *label) {
    // Get text dimensions to work out coordinates to draw the label at
    int bbx, bby, width, height;
    al_get_text_dimensions(label->font, label->text, &bbx, &bby, &width, &height);

    int x = label->x - 0.5 * width;
    int y = label->y - 0.5 * height;

    al_draw_text(label->font, label_colour, x, y, 0, label->text);
}

/*
 * Draw the background colour over the provided label
 */
void clear_label(struct Label *label) {
    int width = al_get_text_width(label->font, label->text);
    int height = al_get_font_line_height(label->font);

    al_draw_filled_rectangle(label->x - 0.5 * width, label->y - 0.5 * height,
                             label->x + 0.5 * width, label->y + 0.5 * height,
                             background_colour);
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

/*
 * Draw the image with the specified filename
 */
void draw_image(char *name, int x, int y, int width, int height) {
    ALLEGRO_BITMAP *bmp = get_bitmap(name);

    al_draw_scaled_bitmap(
        bmp,
        // Source x, y, width and height
        0, 0, al_get_bitmap_width(bmp), al_get_bitmap_height(bmp),
        // Destination x, y, width and height
        x, y, width, height,
        0
    );
}