CFLAGS=-Wall -O2 -ggdb `sdl-config --cflags`
LDFLAGS=`sdl-config --libs` -lSDL_image
all:puertoplayer

clean:
	rm -rf puertoplayer
