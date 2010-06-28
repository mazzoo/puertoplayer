/*
 * puertoplayer
 *
 * a nighttime composition in opensource
 *
 * this is part of the 2010 binkenlights installation
 * in Giesing, Munich, Germany which is called
 *
 *   a.c.a.b. - all colors are beautiful
 *
 * the installation is run by the Chaos Computer Club Munich
 * as part of the puerto giesing
 *
 *
 * license:
 *          GPL v2, see the file LICENSE
 * authors:
 *          Matthias Wenzel - aka - mazzoo
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


int winx = 640;
int winy = 476;

SDL_Surface * s = NULL; /* colored rectangles / windows */
SDL_Surface * p = NULL; /* picture of the frontside of the building */

#define HERTIE_GIESING "hertie_640x476.png"

#define PUERTO_X 24
#define PUERTO_Y 4

uint8_t frame[PUERTO_X * PUERTO_Y * 3];

//#define TEST 1

void refresh_frame(void)
{
	int ix;
	int iy;
	SDL_Rect pixel;

	int off_x = 55;
	int off_y = 50;

#if 0
	pixel.w = winx / PUERTO_X;
	pixel.h = winy / PUERTO_Y;
#else
	pixel.w = 21;
	pixel.h = 69;
#endif

#ifdef TEST
	static int col = 0;
	if (col) col = 0;
	else     col = 1;
#endif
	for (ix=0; ix < PUERTO_X; ix++)
	{
		pixel.x = pixel.w * ix + off_x;
		for (iy=0; iy < PUERTO_Y; iy++)
		{
			pixel.y = pixel.h * iy + off_y;
			SDL_FillRect(s, &pixel, SDL_MapRGB(
			                               s->format,
#ifdef TEST
						       (ix+iy+col)%2 ? 0 : 0xff,
						       (ix+iy+col)%2 ? 0 : 0xff,
						       (ix+iy)%2 ? 0 : 0xff
#else
			                               frame[(ix + iy * PUERTO_X) * 3 + 0],
			                               frame[(ix + iy * PUERTO_X) * 3 + 1],
			                               frame[(ix + iy * PUERTO_X) * 3 + 2]
#endif
						       ));
		}
	}
	SDL_BlitSurface(p, 0, s, 0);
	SDL_Flip(s);
}

void load_hertie_giesing(void)
{
	p = IMG_Load(HERTIE_GIESING);
	if (!p)
	{
		printf("ERROR: IMG_Load(%s): %s\n",
		      HERTIE_GIESING, SDL_GetError());
		exit(1);
	}
#if 0
	int ret;
	ret = SDL_SetAlpha(p, SDL_SRCALPHA, 128);
	if (ret)
	{
		printf("ERROR: SDL_SetAlpha(): %s\n",
		      SDL_GetError());
		exit(1);
	}
	ret = SDL_SetColorKey(p, SDL_SRCCOLORKEY, SDL_MapRGB(s->format, 0xff, 0xff, 0xff));
	if (ret)
	{
		printf("ERROR: SDL_SetColorKey(): %s\n",
		      SDL_GetError());
		exit(1);
	}
#endif
}

int main(int argc, char ** argv)
{
	int ret;
	int f;

	f = open(argv[1], O_RDONLY);
	ret = SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);

	load_hertie_giesing();
	s = SDL_SetVideoMode(winx, winy, 24, SDL_HWSURFACE);

	while (read(f, frame, PUERTO_X * PUERTO_Y * 3) == PUERTO_X * PUERTO_Y * 3)
	{
		refresh_frame();
		SDL_Delay(30);
	}

	SDL_Delay(3000);

	SDL_Quit();
	return 0;
}
