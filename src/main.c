#include <stdio.h>
#include <time.h>

#include <allegro5/allegro.h>

#include "minesweeper.h"
#include "graphics.h"

#define DISPLAY_WIDTH 900
#define DISPLAY_HEIGHT 700

enum AppState {
    MAIN_MENU,
    IN_GAME,
    POST_GAME_MENU,
    ERROR
};

struct App {
    enum AppState state;
    struct Game game;

    struct Button small_game_button;
    struct Button medium_game_button;
    struct Button large_game_button;

    // An array of pointers to buttons for the main menu
    struct Button *main_menu_buttons[3];

    // The button/cell that is current being hovered over
    struct Button *hovered_button;
    int hovered_cell;

    int redraw_required;
};

/*
 * A union to pass parameters to the change_app_state function
 */
union StateChangeParams {
    // This is used when changing to IN_GAME state
    struct {
        int width;
        int height;
        int mine_count;
    } game_settings;

    // This is used when changing to POST_GAME
    int won_game;
};

/*
 * Initialise an App struct by creating the menu buttons and initialising
 * member variables
 */
void init_app(struct App *app) {

    // Create the main menu buttons
    strcpy(app->small_game_button.label, "Small");
    strcpy(app->medium_game_button.label, "Medium");
    strcpy(app->large_game_button.label, "Large");
    app->main_menu_buttons[0] = &(app->small_game_button);
    app->main_menu_buttons[1] = &(app->medium_game_button);
    app->main_menu_buttons[2] = &(app->large_game_button);

    // Set the coordinates of main menu buttons
    for (int i=0; i<3; i++) {
        app->main_menu_buttons[i]->x = DISPLAY_WIDTH / 2;
        app->main_menu_buttons[i]->y = (i + 1) * DISPLAY_HEIGHT / 4;
    }

    app->hovered_button = NULL;
    app->hovered_cell = -1;
}

/*
 * Change the state of the provided app and perform any necessary actions
 * depending on the new state
 */
void change_app_state(struct App *app, enum AppState new_state,
                      union StateChangeParams params) {

    if (new_state == MAIN_MENU) {
        // TODO: Draw background

        // Draw main menu buttons
        for (int i=0; i<3; i++) {
            draw_button(app->main_menu_buttons[i], 0);
        }

        app->redraw_required = 1;
    }

    else if (new_state == IN_GAME) {

        if (init_game(&(app->game), params.game_settings.width,
                      params.game_settings.height,
                      params.game_settings.mine_count, DISPLAY_WIDTH,
                      DISPLAY_HEIGHT)) {

            draw_game(&(app->game));
            app->redraw_required = 1;
        }
        else {
            fprintf(stderr, "Error initialising game\n");
            app->state = ERROR;
            return;
        }
    }

    else if (new_state == POST_GAME_MENU) {
    }

    app->state = new_state;
}

/*
 * Callback function for an allegro mouse button up event. Reveal/toggle flag
 * a cell if in game, or respond to button presses in menus
 */
void handle_click(struct App *app, int mouse_x, int mouse_y, int mouse_button) {
    if (app->state == IN_GAME) {
        int x, y;
        if (get_clicked_cell(&(app->game), mouse_x, mouse_y, &x, &y)) {

            // Left click - reveal the cell
            if (mouse_button == 1) {
                reveal_cell(&(app->game), x, y);
            }
            // Right click - toggle flag
            else if (mouse_button == 2) {
                toggle_flag(&(app->game), x, y);
            }

            draw_game(&(app->game));
            app->redraw_required = 1;

            if (lost_game(&(app->game))) {
                union StateChangeParams params;
                params.won_game = 0;
                change_app_state(app, POST_GAME_MENU, params);
            }

            if (won_game(&(app->game))) {
                union StateChangeParams params;
                params.won_game = 1;
                change_app_state(app, POST_GAME_MENU, params);
            }
        }
    }
    else if (app->state == MAIN_MENU) {

        struct Button *button = get_clicked_button(app->main_menu_buttons, 3,
                                                   mouse_x, mouse_y);
        if (button != NULL) {
            union StateChangeParams params;

            if (button == &(app->small_game_button)) {
                params.game_settings.width = 8;
                params.game_settings.height = 8;
                params.game_settings.mine_count = 10;
            }
            else if (button == &(app->medium_game_button)) {
                params.game_settings.width = 16;
                params.game_settings.height = 16;
                params.game_settings.mine_count = 30;
            }
            change_app_state(app, IN_GAME, params);
        }
    }
}

/*
 * Callback function for a mouse move event. Handle hovering of buttons in the
 * menus and cells in the game
 */
void handle_mouse_move(struct App *app, int mouse_x, int mouse_y) {
    if (app->state == MAIN_MENU) {
        struct Button *button = get_clicked_button(app->main_menu_buttons, 3,
                                                   mouse_x, mouse_y);

        if (button != app->hovered_button) {
            // If there is currently a button being hovered which has now lost
            // focus, unhover it
            if (app->hovered_button != NULL) {
                draw_button(app->hovered_button, 0);
                app->redraw_required = 1;
            }

            // If the mouse is now over a different button, then hover that
            // button
            if (button != NULL) {
                draw_button(button, 1);
                app->redraw_required = 1;
            }

            // Update the currently hovered button - note that if button == NULL
            // this will get updated correctly too
            app->hovered_button = button;
        }
    }
    else if (app->state == IN_GAME) {
        // Note: This is largely the same logic as above, but for hovering cells
        // whilst in game
        int pos, x, y;
        if (get_clicked_cell(&(app->game), mouse_x, mouse_y, &x, &y)) {
            pos = x + y * app->game.width;
        }
        else {
            pos = -1;
        }

        if (pos != app->hovered_cell) {
            if (app->hovered_cell >= 0) {
                int hover_x = app->hovered_cell % app->game.width;
                int hover_y = app->hovered_cell / app->game.width;
                draw_cell(&(app->game), hover_x, hover_y, 0);
                app->redraw_required = 1;
            }
            if (pos >= 0) {
                draw_cell(&(app->game), x, y, 1);
                app->redraw_required = 1;
            }
            app->hovered_cell = pos;
        }
    }
}

int main(int argc, char **args) {
    srand(time(NULL));

    // Initialise allegro related things
    ALLEGRO_DISPLAY *display;
    ALLEGRO_EVENT_QUEUE *event_queue;
    ALLEGRO_TIMER *timer;
    if (!init_allegro(DISPLAY_WIDTH, DISPLAY_HEIGHT, &display, &event_queue,
                      &timer)) {
        return -1;
    }

    // Initialise app and set state to main menu
    struct App app;
    init_app(&app);
    union StateChangeParams params;
    change_app_state(&app, MAIN_MENU, params);

    // Main loop
    while (1) {

        ALLEGRO_EVENT event;
        ALLEGRO_TIMEOUT timeout;
        al_init_timeout(&timeout, 0.06);

        // Wait for an event to be received or for timeout
        bool event_received = al_wait_for_event_until(event_queue, &event,
                                                      &timeout);

        if (event_received) {
            // Close window if close button was pressed
            if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                return 0;
            }

            // Handle mouse clicks
            else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
                handle_click(&app, event.mouse.x, event.mouse.y,
                             event.mouse.button);
            }

            // Handle mouse movement - used to detect when a button is hovered
            else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
                handle_mouse_move(&app, event.mouse.x, event.mouse.y);
            }

            // Timer events signify when the display can be redrawn
            else if (event.type == ALLEGRO_EVENT_TIMER) {
                if (app.redraw_required) {
                    al_flip_display();
                    app.redraw_required = 0;
                }
            }
        }
    }

    return 0;
}