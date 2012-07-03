GMOCK = /opt/gmock-1.6.0

CC = g++-4.7 -std=c++11 -Waddress -Warray-bounds -Wcomment -Wformat -Wreorder -Wreturn-type -Wsequence-point -Wsign-compare -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wvolatile-register-var -Wclobbered -Wempty-body -Wignored-qualifiers -Wsign-compare -Wtype-limits -Wuninitialized -Wunused-parameter -Wunused-but-set-parameter

all:	test exe

exe:
	$(CC) -O2 ps3.c process_status.c processes.c png_textures.c -o ps3 -lGL -lGLU -lpng `sdl-config --cflags --libs`

test:
	$(CC) -g -fprofile-arcs -ftest-coverage -I$(GMOCK)/gtest/include -I$(GMOCK)/include $(GMOCK)/gtest/src/gtest_main.cc test.c process_status.c processes.c -lpthread libgmock.a -o run_test
	./run_test
