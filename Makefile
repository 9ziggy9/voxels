CC=gcc
CFLAGS=-Wall -Wextra -pedantic -fPIC
EXE=./run
RAYLIB_PATH=./raylib/build
RAYFLAGS=-I$(RAYLIB_PATH)/src -L$(RAYLIB_PATH)/src -lraylib -lGL \
				-lm -lpthread -ldl -lrt -lX11
TRASH=$(EXE) libvoxel.so voxel.o

ifdef MEM_TEST
CFLAGS += -DMEM_TEST__
endif

main: main.c libvoxel.so
	$(CC) $(CFLAGS) -o $(EXE) main.c -L. -lvoxel $(RAYFLAGS) -Wl,-rpath,.

# NOTE: this is intended to be compiled once, ran, and forever forgotten.
# Build procecure should dictate that on install, this shared library creates
# a very simple voxel model, bound to requisite textures. After this is done,
# the main executable needn't rely on libvoxel.so anymore. For this reason, we
# should implement an entirely seperate "install" bin.
libvoxel.so: voxel.o
	$(CC) -shared -o libvoxel.so voxel.o $(RAYFLAGS)

voxel.o: voxel.c
	$(CC) $(CFLAGS) -c voxel.c

install_raylib:
	mkdir -p raylib/build
	cd raylib/build && cmake .. -DBUILD_SHARED_LIBS=OFF
	cd raylib/build && make

clean:
	rm -rf $(TRASH)
