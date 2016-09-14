#include <stdio.h>
#include <time.h>
#include <libgen.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "minesweeper.h"
#include "graphics.h"
#include "error.h"

#define DISPLAY_WIDTH 900
#define DISPLAY_HEIGHT 700

#define MAIN_MENU_BUTTON_COUNT 3
#define POST_GAME_MENU_BUTTON_COUNT 3

// The width/height for icons (e.g. flags remaining and timer icos)
#define ICON_SIZE 40

// The horizontal padding for the flags remaining/timer labels
#define FLAG_TIMER_PADDING 10

enum AppState {
    MAIN_MENU,
    IN_GAME,
    POST_GAME_MENU,
    ERROR
};

struct App {
    enum AppState state;
    struct Game game;

    // Main menu buttons and labels
    struct Button small_game_button;
    struct Button medium_game_button;
    struct Button large_game_button;
    struct Label title_label;
    struct Label flags_label;

    // Post-game menu buttons and labels
    struct Button replay_game_button;
    struct Button go_to_main_menu_button;
    struct Button quit_button;
    struct Label game_result_label;

    // An array of pointers to buttons for the menus
    struct Button *main_menu_buttons[MAIN_MENU_BUTTON_COUNT];
    struct Button *post_game_menu_buttons[POST_GAME_MENU_BUTTON_COUNT];

    // The button/cell that is current being hovered over
    struct Button *hovered_button;
    int hovered_cell;

    // Label to show elapsed time
    struct Label timer_label;

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
 * Update the label that shows the number of flags remaning
 */
void update_flags_label(struct App *app) {
    clear_label(&(app->flags_label));
    sprintf(app->flags_label.text, "%d", app->game.flags_remaining);
    draw_label(&(app->flags_label));
    app->redraw_required = 1;
}

/*
 * Update the time elapsed label for the game
 */
void update_game_timer(struct App *app) {
    if (app->state == IN_GAME) {
        static int elapsed_seconds = -1;
        int new_elapsed_seconds = time(NULL)- app->game.timestamp;

        if (new_elapsed_seconds != elapsed_seconds) {
            elapsed_seconds = new_elapsed_seconds;
            clear_label(&(app->timer_label));
            sprintf(app->timer_label.text, "%ds", elapsed_seconds);
            draw_label(&(app->timer_label));
            app->redraw_required = 1;
        }
    }
}

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

    // Create the post-game menu buttons
    strcpy(app->replay_game_button.label, "Play again");
    strcpy(app->go_to_main_menu_button.label, "Main menu");
    strcpy(app->quit_button.label, "Quit");
    app->post_game_menu_buttons[0] = &(app->replay_game_button);
    app->post_game_menu_buttons[1] = &(app->go_to_main_menu_button);
    app->post_game_menu_buttons[2] = &(app->quit_button);

    // Set up labels. Note that the text for game_result_label is set when the
    // game ends, and timer_label and flags_label is set when game starts
    strcpy(app->title_label.text, "Minesweeper");
    set_label_font(&(app->title_label), 60);
    set_label_font(&(app->game_result_label), 40);
    set_label_font(&(app->timer_label), 30);
    set_label_font(&(app->flags_label), 30);

    app->title_label.alignment = ALIGN_CENTER;
    app->game_result_label.alignment = ALIGN_CENTER;
    app->timer_label.alignment = ALIGN_RIGHT;
    app->flags_label.alignment = ALIGN_LEFT;

    // Set the coordinates of the main menu items...
    int spacing = DISPLAY_HEIGHT / (MAIN_MENU_BUTTON_COUNT + 2);
    app->title_label.x = DISPLAY_WIDTH / 2;
    app->title_label.y = spacing;

    for (int i=0; i<MAIN_MENU_BUTTON_COUNT; i++) {
        app->main_menu_buttons[i]->x = DISPLAY_WIDTH / 2;
        app->main_menu_buttons[i]->y = (i + 2) * spacing;
    }

    // ...and the post-game menu items
    spacing = DISPLAY_HEIGHT / (POST_GAME_MENU_BUTTON_COUNT + 2);
    app->game_result_label.x = DISPLAY_WIDTH / 2;
    app->game_result_label.y = spacing;

    for (int i=0; i<3; i++) {
        app->post_game_menu_buttons[i]->x = DISPLAY_WIDTH / 2;
        app->post_game_menu_buttons[i]->y = (i + 2) * spacing;
    }

    // Set the coordinates of the timer and flags labels
    app->timer_label.x = DISPLAY_WIDTH - FLAG_TIMER_PADDING - ICON_SIZE;
    app->timer_label.y = app->timer_label.font_size;

    app->flags_label.x = FLAG_TIMER_PADDING + ICON_SIZE;
    app->flags_label.y = app->flags_label.font_size;

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
        app->hovered_button = NULL;

        draw_background();
        draw_label(&(app->title_label));

        // Draw main menu buttons
        for (int i=0; i<MAIN_MENU_BUTTON_COUNT; i++) {
            draw_button(app->main_menu_buttons[i], 0);
        }

        app->redraw_required = 1;
    }

    else if (new_state == IN_GAME) {
        if (init_game(&(app->game), params.game_settings.width,
                      params.game_settings.height,
                      params.game_settings.mine_count, DISPLAY_WIDTH,
                      DISPLAY_HEIGHT, GRID_PADDING, CELL_PADDING)) {

            draw_background();
            draw_game(&(app->game));

            // Draw flag icon next to flags remaining label
            draw_image("flag.png", FLAG_TIMER_PADDING,
                       app->flags_label.y - 0.5 * ICON_SIZE, ICON_SIZE,
                       ICON_SIZE);
            strcpy(app->flags_label.text, "0");
            update_flags_label(app);

            // Draw clock icon next to timer label
            draw_image("clock.png",
                       DISPLAY_WIDTH - FLAG_TIMER_PADDING - ICON_SIZE,
                       app->timer_label.y - 0.5 * ICON_SIZE, ICON_SIZE,
                       ICON_SIZE);

            app->redraw_required = 1;
        }
        else {
            exit_app(EXIT_FAILURE);
        }
    }

    else if (new_state == POST_GAME_MENU) {
        app->hovered_button = NULL;

        // Shade over the grid
        shade_screen(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

        // Show game result label
        char *result_str = (params.won_game ? "You won!" : "You lost");
        strcpy(app->game_result_label.text, result_str);
        draw_label(&(app->game_result_label));

        // Draw menu buttons
        for (int i=0; i<POST_GAME_MENU_BUTTON_COUNT; i++) {
            draw_button(app->post_game_menu_buttons[i], 0);
        }
        app->redraw_required = 1;
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

            // Left click
            if (mouse_button == 1) {
                int cell = get_cell(&(app->game), x, y);

                // If this cell is unknown then reveal it
                if (cell == CELL_TYPE_UNKNOWN){
                    reveal_cell(&(app->game), x, y);
                }
                // If the cell is not revealed and not a flag, then 'click' all
                // neighbouring cells
                else if (cell != CELL_TYPE_FLAG) {
                    reveal_neighobouring_cells(&(app->game), x, y);
                }
            }
            // Right click - toggle flag
            else if (mouse_button == 2) {
                toggle_flag(&(app->game), x, y);
                update_flags_label(app);
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

        struct Button *button = get_clicked_button(app->main_menu_buttons,
                                                   MAIN_MENU_BUTTON_COUNT,
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
            else if (button == &(app->large_game_button)) {
                params.game_settings.width = 30;
                params.game_settings.height = 16;
                params.game_settings.mine_count = 99;
            }
            change_app_state(app, IN_GAME, params);
        }
    }
    else if (app->state == POST_GAME_MENU) {
        struct Button *button = get_clicked_button(app->post_game_menu_buttons,
                                                   POST_GAME_MENU_BUTTON_COUNT,
                                                   mouse_x, mouse_y);
        if (button != NULL) {
            union StateChangeParams params;

            if (button == &(app->replay_game_button)) {
                params.game_settings.width = app->game.width;
                params.game_settings.height = app->game.height;
                params.game_settings.mine_count = app->game.mine_count;
                change_app_state(app, IN_GAME, params);
            }
            else if (button == &(app->go_to_main_menu_button)) {
                change_app_state(app, MAIN_MENU, params);
            }
            else if (button == &(app->quit_button)) {
                exit_app(EXIT_SUCCESS);
            }
        }
    }
}

/*
 * Callback function for a mouse move event. Handle hovering of buttons in the
 * menus and cells in the game
 */
void handle_mouse_move(struct App *app, int mouse_x, int mouse_y) {
    if (app->state == MAIN_MENU || app->state == POST_GAME_MENU) {

        // Work out which buttons to check for hovering
        struct Button **buttons;
        int button_count;
        if (app->state == MAIN_MENU) {
            buttons = app->main_menu_buttons;
            button_count = MAIN_MENU_BUTTON_COUNT;
        }
        else {
            buttons = app->post_game_menu_buttons;
            button_count = POST_GAME_MENU_BUTTON_COUNT;
        }

        struct Button *button = get_clicked_button(buttons, button_count,
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
    char asset_dir[200];
    sprintf(asset_dir, "%s/%s", dirname(args[0]), "assets");
    ALLEGRO_DISPLAY *display;
    ALLEGRO_EVENT_QUEUE *event_queue;
    ALLEGRO_TIMER *timer;
    if (!init_allegro(DISPLAY_WIDTH, DISPLAY_HEIGHT, &display, &event_queue,
                      &timer, asset_dir)) {
        exit_app(EXIT_FAILURE);
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
                exit_app(EXIT_SUCCESS);
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

            // Timer events signify when the display can be redrawn and when
            // time elapsed can be updated
            else if (event.type == ALLEGRO_EVENT_TIMER) {
                update_game_timer(&app);

                if (app.redraw_required) {
                    al_flip_display();
                    app.redraw_required = 0;
                }
            }
        }
    }

    return 0;
}