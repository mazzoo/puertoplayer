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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


int winx = 640;
int winy = 476;

#define HERTIE_GIESING "hertie_640x476.png"
#define ICON_LOVE      "i_can_prit_love.png"

#define ACAB_IP      (  127  \
                    |(    0  \
               << 8)|(    0  \
               <<16)|(    1  \
               <<24))

int a; /* socket to acab */

SDL_Surface * s    = NULL; /* colored rectangles / windows */
SDL_Surface * p    = NULL; /* picture of the frontside of the building */
SDL_Surface * icon = NULL; /* icon of love */

int mode = 0; /* 0: play local argv[1] file
	       * 1: connect live to gigargoyle @ puerto giesing
	       */

#define PUERTO_X 24
#define PUERTO_Y 4

uint8_t frame[PUERTO_X * PUERTO_Y * 3];

uint8_t buf[4096];

SDL_Event e;


void refresh_frame(void)
{
	int ix;
	int iy;
	SDL_Rect pixel;

	int off_x = 55;
	int off_y = 50;

	pixel.w = 21;
	pixel.h = 69;

	for (ix=0; ix < PUERTO_X; ix++)
	{
		pixel.x = pixel.w * ix + off_x;
		for (iy=0; iy < PUERTO_Y; iy++)
		{
			pixel.y = pixel.h * iy + off_y;
			SDL_FillRect(s, &pixel, SDL_MapRGB(
			                               s->format,
			                               frame[(ix + iy * PUERTO_X) * 3 + 0],
			                               frame[(ix + iy * PUERTO_X) * 3 + 1],
			                               frame[(ix + iy * PUERTO_X) * 3 + 2]
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
	icon = IMG_Load(ICON_LOVE);
	if (!icon)
	{
		printf("ERROR: IMG_Load(%s): %s\n",
		      ICON_LOVE, SDL_GetError());
		exit(1);
	}
}

void init_socket(void)
{
	int ret;
	struct sockaddr_in sa;
	a = socket(AF_INET, SOCK_STREAM, 0);
	if (a < 0)
	{
		printf("ERROR: connect(): %s\n", strerror(errno));
		exit(1);
	}
	memset(&sa, 0, sizeof(sa));
	sa.sin_family        = AF_INET;
	sa.sin_port          = htons(80);
	sa.sin_addr.s_addr   = ACAB_IP;
	ret = connect(a, (struct sockaddr *) &sa, sizeof(sa));
	if (ret < 0)
	{
		printf("ERROR: connect(): %s\n", strerror(errno));
		exit(1);
	}
}


int main(int argc, char ** argv)
{
	int ret;
	int f = -1;

	int in;

	if (argv[1])
		f = open(argv[1], O_RDONLY);
	else{
		mode = 1;
		init_socket();
	}

	ret = SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);

	load_hertie_giesing();
	s = SDL_SetVideoMode(winx, winy, 24, SDL_HWSURFACE);

	SDL_WM_SetCaption( "puertoplayer - ACAB - all colours are beautiful", "ACAB" );
	SDL_WM_SetIcon(icon, NULL);

	if (mode)
		in = a;
	else
		in = f;

	while (read(in, frame, PUERTO_X * PUERTO_Y * 3) == PUERTO_X * PUERTO_Y * 3)
	{
		refresh_frame();
		SDL_Delay(1);
		SDL_PollEvent(&e );
		if( e.type == SDL_QUIT )
			exit(0);
	}

	SDL_Delay(2342);

	SDL_Quit();
	return 0;
}
