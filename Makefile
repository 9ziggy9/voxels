CC=gcc
CFLAGS=-Wall -Wextra -pedantic -fPIC
RAYLIB_PATH=./raylib/build/raylib
RAYLIB_STATIC_LIB=$(RAYLIB_PATH)/libraylib.a
EXE=./run
SRCS = world.c voxel.c noise.c camera.c
OBJS = $(SRCS:.c=.o)
TRASH=$(EXE) $(OBJS) libvoxel.so

.PHONY: clean install_raylib

main: $(OBJS) libvoxel.so main.c
	$(CC) $(CFLAGS) -o $(EXE) main.c -L. noise.o camera.o \
	-lvoxel $(RAYLIB_STATIC_LIB) -lGL -lm -lpthread -ldl -lrt -lX11 -Wl,-rpath,.

libvoxel.so: voxel.o
	$(CC) -shared -o libvoxel.so voxel.o

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -I$(RAYLIB_PATH)/src

install_raylib:
	mkdir -p raylib/build
	cd raylib/build && cmake .. -DBUILD_SHARED_LIBS=OFF
	cd raylib/build && make

clean:
	rm -rf $(TRASH)
