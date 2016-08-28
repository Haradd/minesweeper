#ifndef GRAPHICS_H
#define GRAPHICS_H

int init_allegro();
void draw_grid(struct Grid *grid);
void draw_cell(struct Grid *grid, int x, int y);

#endif