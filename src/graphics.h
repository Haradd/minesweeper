#ifndef GRAPHICS_H
#define GRAPHICS_H

#define MAX_BUTTON_LENGTH 20
#define MAX_LABEL_LENGTH 20

struct Button {
    char label[MAX_BUTTON_LENGTH];

    // The coordinates of the CENTER of the button
    int x;
    int y;
};

struct Label {
    char text[MAX_LABEL_LENGTH];
    int font_size;

    // The coordinates of the CENTER of the text
    int x;
    int y;
};

int init_allegro(int width, int height, ALLEGRO_DISPLAY **display,
                 ALLEGRO_EVENT_QUEUE **event_queue, ALLEGRO_TIMER **timer);
void draw_cell(struct Game *game, int x, int y, int hovered);
void draw_game(struct Game *game);
void draw_button(struct Button *button, int hovered);
void draw_label(struct Label *label);
struct Button *get_clicked_button(struct Button **buttons, int count, int mouse_x,
                                  int mouse_y);
int get_clicked_cell(struct Game *game, int mouse_x, int mouse_y, int *x_ptr,
                     int *y_ptr);
void shade_screen(int x1, int y1, int x2, int y2);
void draw_background();

#endif