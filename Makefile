CC = g++ -std=gnu++0x -O -Wall

all:
	$(CC) ps3.c process_status.c png_textures.c -o ps3 -lGL -lGLU -lpng `sdl-config --cflags --libs`
