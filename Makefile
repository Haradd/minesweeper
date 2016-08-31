addons = allegro-5.0 allegro_main-5.0 allegro_primitives-5.0 allegro_font-5.0 allegro_ttf-5.0
files = src/main.c src/minesweeper.c src/graphics.c

default: $(files)
	gcc -g -o minesweeper $(files) $(shell pkg-config --cflags --libs $(addons))
