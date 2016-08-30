#ifndef GRAPHICS_H
#define GRAPHICS_H

#define MAX_BUTTON_LENGTH 10

struct Button {
    int id;
    char label[MAX_BUTTON_LENGTH];

    // The coordinates of the CENTER of the button
    int x;
    int y;
};

int init_allegro();
void draw_game(struct Game *game);
void create_button(struct Button *button, int id, char *label, int x, int y);
void draw_button(struct Button *button);

int get_clicked_button(struct Button *buttons, int count, int mouse_x,
                       int mouse_y);
int get_clicked_cell(struct Game *game, int mouse_x, int mouse_y, int *x_ptr,
                     int *y_ptr);

#endif