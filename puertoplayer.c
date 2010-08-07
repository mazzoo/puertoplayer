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


#define ICON_LOVE    "i_can_prit_love.png"

#define HERTIE_IMAGE 2 /* default index into hertie_images[] */

struct image_info
{
	const char * fname;
	const int    x;
	const int    y;
	const int    off_x;
	const int    off_y;
	const int    win_x;
	const int    win_y;
	const int    stride_x;
	const int    stride_y;
	const int    stride_x_mod;
	const int    stride_y_mod;
};

struct image_info hertie_images[] =
{
	{
		"hertie_640x476.png",
		640, 476,
		55, 50,
		21, 69,
		0, 0,
		0, 0
	},
	{
		"hertie_1328x960.png",
		1328,  960,          /* x * y  */
		 284,  167,          /* offset */
		  31,  116,          /* window */
		  11,    0,          /* stride */
		   2,    0           /* stride every n */
	},
	{
		"hertie_800x578.png",
		 800, 578,
		 170, 100,
		  17,  69,
		  10,   0,
		   2,   0
	},
	{
		"hertie_640x463.png",
		640, 463,
		138,  80,
		 14,  55,
		  7, 0,
		  2, 0
	}
};

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

	int off_x = hertie_images[HERTIE_IMAGE].off_x;
	int off_y = hertie_images[HERTIE_IMAGE].off_y;

	pixel.w = hertie_images[HERTIE_IMAGE].win_x;
	pixel.h = hertie_images[HERTIE_IMAGE].win_y;

	for (ix=0; ix < PUERTO_X; ix++)
	{
		if (hertie_images[HERTIE_IMAGE].stride_x_mod)
			if (!(ix % hertie_images[HERTIE_IMAGE].stride_x_mod))
				off_x += hertie_images[HERTIE_IMAGE].stride_x;

		pixel.x = pixel.w * ix + off_x;
		for (iy=0; iy < PUERTO_Y; iy++)
		{
			if (hertie_images[HERTIE_IMAGE].stride_y_mod)
				if (!(iy % hertie_images[HERTIE_IMAGE].stride_y_mod))
					off_y += hertie_images[HERTIE_IMAGE].stride_y;

			pixel.y = pixel.h * iy + off_y;
			SDL_FillRect(s,
			             &pixel,
			             SDL_MapRGB(
			                        s->format,
			                        frame[(ix + iy * PUERTO_X) * 3 + 0],
			                        frame[(ix + iy * PUERTO_X) * 3 + 1],
			                        frame[(ix + iy * PUERTO_X) * 3 + 2]
			                       )
			            );
		}
	}
	SDL_BlitSurface(p, 0, s, 0);
	SDL_Flip(s);
}

void load_hertie_giesing(void)
{
	p = IMG_Load(hertie_images[HERTIE_IMAGE].fname);
	if (!p)
	{
		printf("ERROR: IMG_Load(%s): %s\n",
		      hertie_images[HERTIE_IMAGE].fname, SDL_GetError());
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
	s = SDL_SetVideoMode(
	        hertie_images[HERTIE_IMAGE].x,
	        hertie_images[HERTIE_IMAGE].y,
	        24, SDL_HWSURFACE | SDL_HWACCEL | SDL_ASYNCBLIT);

	if (!s){
		printf("ERROR: couldn't SDL_SetVideoMode(): %s\n", SDL_GetError());
		exit(1);
	}

	SDL_WM_SetCaption( "puertoplayer - ACAB - all colours are beautiful", "ACAB" );
	SDL_WM_SetIcon(icon, NULL);

	if (mode)
		in = a;
	else
		in = f;

	while (read(in, frame, PUERTO_X * PUERTO_Y * 3) == PUERTO_X * PUERTO_Y * 3)
	{
		refresh_frame();
		if (!mode)
			usleep(25000);
		SDL_PollEvent( &e );
		if (e.type == SDL_KEYDOWN)
		{
			if (e.key.keysym.sym == SDLK_ESCAPE)
				exit(0);
			if (e.key.keysym.sym == SDLK_q)
				exit(0);
		}
		if( e.type == SDL_QUIT )
			exit(0);

	}

	SDL_Delay(2342);

	SDL_Quit();
	return 0;
}
