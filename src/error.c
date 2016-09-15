#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*
 * Exit the application with the specified status
 *
 * TODO: Free up memory and destory allegro things before exiting
 */
void exit_app(int status) {
    exit(status);
}

/*
 * Print the program name followed by a colon, a space and the provided error
 * message
 */
void print_error(const char *format, ...) {
    fprintf(stderr, "minesweeper: ");
    va_list arg_ptr;
    va_start(arg_ptr, format);
    vfprintf(stderr, format, arg_ptr);
    fprintf(stderr, "\n");
}
