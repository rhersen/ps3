CC = g++ -std=gnu++0x -O2 -Waddress -Warray-bounds -Wcomment -Wformat -Wreorder -Wreturn-type -Wsequence-point -Wsign-compare -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wvolatile-register-var -Wclobbered -Wempty-body -Wignored-qualifiers -Wsign-compare -Wtype-limits -Wuninitialized -Wunused-parameter -Wunused-but-set-parameter

all:
	$(CC) ps3.c process_status.c png_textures.c -o ps3 -lGL -lGLU -lpng `sdl-config --cflags --libs`
