CC = gcc -O -Waggregate-return -Wcast-align  -Wcast-qual  -Wchar-subscripts  -Wcomment  -Wformat  -Wimplicit -Wimplicit-int -Wimplicit-function-declaration  -Wimport -Werror-implicit-function-declaration -Winline  -Wlong-long -Wmain  -Wmissing-declarations  -Wmissing-noreturn -Wmultichar  -Wno-import  -Wpacked -Wredundant-decls -Wreturn-type -Wsign-compare -Wswitch -Wtrigraphs -Wundef  -Wuninitialized  -Wunknown-pragmas -Wunused -Wunused-function -Wunused-label -Wunused-parameter -Wunused-variable -Wunused-value -Wwrite-strings

all:
	$(CC) ps3.c process_status.c png_textures.c -o ps3 -lGL -lGLU -lpng `sdl-config --cflags --libs`
