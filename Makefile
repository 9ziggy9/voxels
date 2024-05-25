CC=gcc
CFLAGS=-Wall -Wextra -pedantic -fPIC
RAYLIB_PATH=./raylib/build/raylib
RAYLIB_STATIC_LIB=$(RAYLIB_PATH)/libraylib.a
EXE=./run
TRASH=$(EXE) libvoxel.so voxel.o

.PHONY: clean install_raylib

main: main.c libvoxel.so
	$(CC) $(CFLAGS) -o $(EXE) main.c -L. -lvoxel $(RAYLIB_STATIC_LIB) -lGL \
	-lm -lpthread -ldl -lrt -lX11 -Wl,-rpath,.

libvoxel.so: voxel.o
	$(CC) -shared -o libvoxel.so voxel.o

voxel.o: voxel.c
	$(CC) $(CFLAGS) -c voxel.c -I$(RAYLIB_PATH)/src

install_raylib:
	mkdir -p raylib/build
	cd raylib/build && cmake .. -DBUILD_SHARED_LIBS=OFF
	cd raylib/build && make

clean:
	rm -rf $(TRASH)
