/*
 * Dan Aloni <alonid@gmail.com>, 2014 (c)
 *
 * This code is licensed under the GNU General Public License
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <assert.h>
#include <sys/time.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <time.h>

#include "scene.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

static int dblBuf[] = { GLX_RGBA,
	GLX_RED_SIZE, 1,
	GLX_GREEN_SIZE, 1,
	GLX_BLUE_SIZE, 1,
	GLX_DEPTH_SIZE, 12,
	GLX_DOUBLEBUFFER,
	None
};

static Atom wmDeleteWindow;
static Display *dpDisplay;
static Window win;

static double time_of_day_float(void)
{
	struct timeval tv;
	int ret;

	ret = gettimeofday(&tv, NULL);
	assert(ret == 0);

	return ((double)tv.tv_sec) + ((double)tv.tv_usec/1000000);
}

static void redraw_scene(time_t time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	scene_display(time);

	glXSwapBuffers(dpDisplay, win);
}

static void init_window(int argc, char **argv)
{
	XVisualInfo *xvVisualInfo;
	Colormap cmColorMap;
	XSetWindowAttributes winAttributes;
	GLXContext glXContext;

	dpDisplay = XOpenDisplay(NULL);
	if (dpDisplay == NULL) {
		printf("Could not open display!\n\r");
		exit(0);
	}

	if (!glXQueryExtension(dpDisplay, NULL, NULL)) {
		printf("X server has no GLX extension.\n\r");
		exit(0);
	}

	/* Grab a doublebuffering RGBA visual as defined in dblBuf */
	xvVisualInfo =
		glXChooseVisual(dpDisplay, DefaultScreen(dpDisplay), dblBuf);
	if (xvVisualInfo == NULL) {
		printf
			("No double buffering RGB visual with depth buffer available.\n\r");
		exit(0);
	}

	/* Create a window context */
	glXContext = glXCreateContext(dpDisplay, xvVisualInfo, None, True);
	if (glXContext == NULL) {
		printf("Could not create rendering context.\n\r");
		exit(0);
	}

	cmColorMap = XCreateColormap(dpDisplay,
				     RootWindow(dpDisplay, xvVisualInfo->screen),
				     xvVisualInfo->visual, AllocNone);
	winAttributes.colormap = cmColorMap;
	winAttributes.border_pixel = 0;
	winAttributes.event_mask =
		ExposureMask | ButtonPressMask | ButtonReleaseMask |
		PointerMotionMask | StructureNotifyMask | KeyPressMask;

	const char *xscreensaver_window = getenv("XSCREENSAVER_WINDOW");
	if (xscreensaver_window) {
		/* Running as a screen saver, take Window ID from environment */
		char *end;

		win = (Window)strtoul(xscreensaver_window, &end, 0);
	} else {
		win = XCreateWindow(dpDisplay, RootWindow(dpDisplay, xvVisualInfo->screen), 0, 0,
				    640,
				    480,
				    0,
				    xvVisualInfo->depth,
				    InputOutput,
				    xvVisualInfo->visual,
				    CWBorderPixel | CWColormap | CWEventMask,
				    &winAttributes);

		XSetStandardProperties(dpDisplay,
				       win,
				       "24clock",
				       "24clock", None, (char **)argv, (int)argc, NULL);
	}

	glXMakeCurrent(dpDisplay, win, glXContext);

	/* Make the new window the active window. */
	XMapWindow(dpDisplay, win);
}

static void main_loop(void)
{
	XEvent event;

	while (1) {
		time_t last_time = 0;

		do {
			while (!XPending(dpDisplay))
			{
				double now = time_of_day_float();
				time_t current_time = round(now);

				if (current_time != last_time) {
					redraw_scene(current_time);
					last_time = current_time;
				} else {
					struct pollfd pfd = {
						.events = POLLIN,
						.fd = XConnectionNumber(dpDisplay),
					};

					/*
					 * Sleep at least a milisecond until the next
					 * second boundary.
					 */
					double now = time_of_day_float();
					unsigned long i = ((now - floor(now)) * 1000);
					if (i >= 1000)
						i = 1000;
					i = 1000 - i;
					if (i == 0)
						i = 1;
					poll(&pfd, 1, i);
				}
			}

			XNextEvent(dpDisplay, &event);

			switch (event.type) {
			case Expose:
				break;

			case ConfigureNotify: {
				/* Resize event */
				scene_reshape(event.xconfigure.width,
						event.xconfigure.height);
				break;
			}

			case ClientMessage:
				if (event.xclient.data.l[0] == wmDeleteWindow) {
					exit(0);
				}
				break;
			}
		} while (XPending(dpDisplay));
	}
}

int main(int argc, char **argv)
{
	XWindowAttributes winattr;

	init_window(argc, argv);

	XGetWindowAttributes(dpDisplay, win, &winattr);

	scene_init(winattr.width, winattr.height);

	main_loop();

	return 1;
}
