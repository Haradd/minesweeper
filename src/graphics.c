#include <stdio.h>
#include <string.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>

#include "minesweeper.h"
#include "graphics.h"
#include "error.h"

#define GRAPHICS_FPS 30

#define FONT_NAME "DejaVuSans.ttf"
#define TITLE_FONT_SIZE 20
#define BUTTON_FONT_SIZE 30

// The vertical space between the text and the sides of the button
#define BUTTON_PADDING 30

// The radius for the rounded rectangle drawn for a cell as percentage
// (100% = Circle)
#define GRID_CELL_RADIUS 0.25

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
char assets_path[200];
char font_path[200];

struct BitmapContainer bitmap_container;

/*
 * Return the full path to a file in the assets directory
 */
const char *get_asset_path(const char *filename) {
    char *path = malloc(strlen(assets_path) + strlen(filename) + 1);
    sprintf(path, "%s%s", assets_path, filename);
    return path;
}

/*
 * Initialise allegro and any allegro addons, and create a display and event
 * queue. Return 1 if successful, 0 otherwise
 */
int init_allegro(int width, int height, ALLEGRO_DISPLAY **display,
                 ALLEGRO_EVENT_QUEUE **event_queue, ALLEGRO_TIMER **timer) {

    // Initialise the bitmap collection
    bitmap_container.count = 0;

    if (!al_init()) {
        print_error("Failed to initialise allegro");
        return 0;
    }
    if (!al_init_primitives_addon()) {
        print_error("Failed to initialise primitives addon");
        return 0;
    }

    if (!al_init_image_addon()) {
        print_error("Failed to initialise image addon");
        return 0;
    }

    al_init_font_addon();
    if (!al_init_ttf_addon()) {
        print_error("Failed to initialise TTF font addon");
        return 0;
    }

    if (!al_install_mouse()) {
        print_error("Failed to install mouse");
        return 0;
    }

    // Create the display
    *display = al_create_display(width, height);
    if (!(*display)) {
        print_error("Failed to create display");
        return 0;
    }

    // Create and start the timer
    *timer = al_create_timer(1.0 / GRAPHICS_FPS);
    if (!(*timer)) {
        print_error("Failed to create timer");
        return 0;
    }
    al_start_timer(*timer);

    // Create and register the event queue
    *event_queue = al_create_event_queue();
    if (!(*event_queue)) {
        print_error("Failed to create event queue");
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

    // Set assets path
    ALLEGRO_PATH *assets_path_al = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
    ALLEGRO_PATH *assets_path_relative = al_create_path_for_directory("assets");
    al_join_paths(assets_path_al, assets_path_relative);
    strcpy(assets_path, al_path_cstr(assets_path_al, ALLEGRO_NATIVE_PATH_SEP));
    al_destroy_path(assets_path_al);
    al_destroy_path(assets_path_relative);

    // Set font path and load fonts
    strcpy(font_path, get_asset_path(FONT_NAME));
    title_font = al_load_ttf_font(font_path, TITLE_FONT_SIZE, 0);
    button_font = al_load_ttf_font(font_path, BUTTON_FONT_SIZE, 0);

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
    ALLEGRO_BITMAP *bmp = al_load_bitmap(get_asset_path(name));

    if (bmp == NULL) {
        char message_part[] = "Failed to load ";
        char *message = malloc(strlen(message_part) + strlen(name) + 1);
        sprintf(message, "%s%s", message_part, name);
        print_error(message);
        free(message);

        exit_app(EXIT_FAILURE);
    }

    // Store the name and bitmap
    strcpy(bitmap_container.names[bitmap_container.count], name);
    bitmap_container.bitmaps[bitmap_container.count] = bmp;
    bitmap_container.count++;

    return bmp;
}

/*
 * Calculate the coordinates of the corners of the rectangle for a cell.
 * Note that this is the coordinates of the visible part, i.e. not including
 * padding
 */
void get_cell_rect(struct Game *game, int x, int y, int *x1, int *y1, int *x2,
                   int *y2) {

    *x1 = game->x_padding + x * (game->cell_size + 2 * game->cell_padding)
          + game->cell_padding;
    *y1 = game->y_padding + y * (game->cell_size + 2 * game->cell_padding)
          + game->cell_padding;
    *x2 = *x1 + game->cell_size;
    *y2 = *y1 + game->cell_size;
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

    // Make the cell background colour a bit darker if the cell is being hovered
    if (hovered) {
        unsigned char r, g, b;
        al_unmap_rgb(cell_colour, &r, &g, &b);
        int delta = -20;
        r += delta;
        g += delta;
        b += delta;

        cell_colour = al_map_rgb(r, g, b);
    }

    int dx1, dy1, dx2, dy2;
    get_cell_rect(game, x, y, &dx1, &dy1, &dx2, &dy2);

    // Draw cell background colour
    float radius = 0.5 * game->cell_size * GRID_CELL_RADIUS;
    al_draw_filled_rounded_rectangle(dx1, dy1, dx2, dy2, radius, radius,
                                     cell_colour);

    // Draw text if text has been specified
    if (text != 0) {
        char string[] = {text};
        al_draw_text(cell_font, text_colour, 0.5 * (dx1 + dx2), dy1,
                     ALLEGRO_ALIGN_CENTRE, string);
    }

    // Draw an image if image_name has been set
    if (strlen(image_name) > 0) {
        draw_image(image_name, dx1, dy1, dx2 - dx1, dy2 - dy1);
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
 * Calculate the coordinates of the rectangle for a given label (see
 * get_button_rect)
 */
void get_label_rect(struct Label *label, int *x1, int *y1, int *x2, int *y2) {
    int width = al_get_text_width(label->font, label->text);
    int height = al_get_font_line_height(label->font);

    // The y-coordinate is always at the center of the label, so subtract half
    // the height
    *y1 = label->y - 0.5 * height;

    // Calculate x1 based on the alignment
    switch (label->alignment) {
        case ALIGN_LEFT:
            *x1 = label->x;
            break;
        case ALIGN_CENTER:
            *x1 = label->x - 0.5 * width;
            break;
        case ALIGN_RIGHT:
            *x1 = label->x - width;
            break;
    }

    *x2 = *x1 + width;
    *y2 = *y1 + height;
}

/*
 * Draw the provided label
 */
void draw_label(struct Label *label) {
    int x1, y1, x2, y2;
    get_label_rect(label, &x1, &y1, &x2, &y2);
    al_draw_text(label->font, label_colour, x1, y1, 0, label->text);
}

/*
 * Draw the background colour over the provided label
 */
void clear_label(struct Label *label) {
    int x1, y1, x2, y2;
    get_label_rect(label, &x1, &y1, &x2, &y2);
    al_draw_filled_rectangle(x1, y1, x2, y2, background_colour);
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
    float total_cell_size = game->cell_size + 2 * game->cell_padding;
    if (x_offset >= 0 && x_offset < game->width * total_cell_size &&
        y_offset >= 0 && y_offset < game->height * total_cell_size) {

        int x = x_offset / total_cell_size;
        int y = y_offset / total_cell_size;

        // Check that the mouse is actually within the cell, not within the
        // padding around the cell
        int x1, y1, x2, y2;
        get_cell_rect(game, x, y, &x1, &y1, &x2, &y2);
        if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1 && mouse_y <= y2) {
            *x_ptr = x;
            *y_ptr = y;
            return 1;
        }
    }

    return 0;
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