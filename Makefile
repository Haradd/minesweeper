addons = allegro-5.0 allegro_main-5.0 allegro_primitives-5.0

default: main.c minesweeper.c graphics.c
	gcc -o minesweeper main.c minesweeper.c graphics.c $(shell pkg-config --cflags --libs $(addons))
