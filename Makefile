all:
	gcc -o mish `pkg-config --cflags --libs glib-2.0 readline` main.c
