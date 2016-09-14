#ifndef GRAPHICS_H
#define GRAPHICS_H

#define MAX_BUTTON_LENGTH 20
#define MAX_LABEL_LENGTH 20

// The minimum padding between the game grid and the edges of the display
#define GRID_PADDING 30

struct Button {
    char label[MAX_BUTTON_LENGTH];

    // The coordinates of the CENTER of the button
    int x;
    int y;
};

// An enum to contain the possible options for label alignment
enum LabelAlignment {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
};

struct Label {
    char text[MAX_LABEL_LENGTH];

    enum LabelAlignment alignment;

    // The x-coordinate to draw the label at. This is the point at the left,
    // center of right of the label depending on the value of alignment
    int x;
    int y;  // The y-coordinate of the center of the label

    int font_size;
    ALLEGRO_FONT *font;
};

int init_allegro(int width, int height, ALLEGRO_DISPLAY **display,
                 ALLEGRO_EVENT_QUEUE **event_queue, ALLEGRO_TIMER **timer,
                 char *asset_dir_p);
ALLEGRO_BITMAP *get_bitmap(char *name);
void draw_cell(struct Game *game, int x, int y, int hovered);
void draw_game(struct Game *game);
void draw_button(struct Button *button, int hovered);
void set_label_font(struct Label *label, int font_size);
void draw_label(struct Label *label);
void clear_label(struct Label *label);
struct Button *get_clicked_button(struct Button **buttons, int count, int mouse_x,
                                  int mouse_y);
int get_clicked_cell(struct Game *game, int mouse_x, int mouse_y, int *x_ptr,
                     int *y_ptr);
void shade_screen(int x1, int y1, int x2, int y2);
void draw_background();
void draw_image(char *name, int x, int y, int width, int height);

#endif