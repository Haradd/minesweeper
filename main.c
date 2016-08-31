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
};

/*
 * Initialise an App struct by creating the menu buttons and setting the state
 */
void init_app(struct App *app) {
    app->state = MAIN_MENU;

    // // Create the main menu buttons
    strcpy(app->small_game_button.label, "Small");
    strcpy(app->medium_game_button.label, "Medium");
    strcpy(app->large_game_button.label, "Large");
    app->main_menu_buttons[0] = &(app->small_game_button);
    app->main_menu_buttons[1] = &(app->medium_game_button);
    app->main_menu_buttons[2] = &(app->large_game_button);

     for (int i=0; i<3; i++) {
        app->main_menu_buttons[i]->x = DISPLAY_WIDTH / 2;
        app->main_menu_buttons[i]->y = (i + 1) * DISPLAY_HEIGHT / 4;

        draw_button(app->main_menu_buttons[i]);
     }
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

            if (lost_game(&(app->game))) {
                printf("You hit a mine!\n");
                app->state = POST_GAME_MENU;
            }

            if (won_game(&(app->game))) {
                printf("You won!\n");
                app->state = POST_GAME_MENU;
            }
        }
    }
    else  if (app->state == MAIN_MENU) {

        struct Button *button = get_clicked_button(app->main_menu_buttons, 3,
                                                   mouse_x, mouse_y);
        if (button != NULL) {
            int width, height, mine_count;

            if (button == &(app->small_game_button)) {
                width = 8;
                height = 8;
                mine_count = 10;
            }
            else if (button == &(app->medium_game_button)) {
                width = 16;
                height = 16;
                mine_count = 30;
            }

            // Start game
            if (!init_game(&(app->game), width, height, mine_count,
                           DISPLAY_WIDTH, DISPLAY_HEIGHT)) {

                fprintf(stderr, "Error initialising game\n");
                app->state = ERROR;
            }
            draw_game(&(app->game));
            app->state = IN_GAME;
        }
    }
}

int main(int argc, char **args) {
    srand(time(NULL));

    ALLEGRO_DISPLAY *display;
    ALLEGRO_EVENT_QUEUE *event_queue;
    if (!init_allegro(DISPLAY_WIDTH, DISPLAY_HEIGHT, &display, &event_queue)) {
        return -1;
    }

    struct App app;
    init_app(&app);

    al_flip_display();

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
        }
    }

    return 0;
}