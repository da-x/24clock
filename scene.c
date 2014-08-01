/*
 * Dan Aloni <alonid@gmail.com>, 2014 (c)
 *
 * This code is licensed under the GNU General Public License
 */

#include <GL/gl.h>
#include <GL/glu.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <sched.h>
#include <stdarg.h>
#include <assert.h>

#include "scene.h"

typedef struct GL3d_s {
	GLdouble x, y, z;
} GL3d;

typedef enum {
	LED_SHARP_START=1,
	LED_SHARP_END=2,
	LED_SHARP_START_LEFT=4,
	LED_SHARP_START_RIGHT=8,
	LED_SHARP_END_RIGHT=16,
	LED_SHARP_END_LEFT=32,
} led_flags_t;

#define LED_WIDTH 2

typedef struct led_s {
	unsigned long flags;
	double start;
	double shift;
	double end;
} led_t;

#define MATRIX_STACK_DEPTH 40

static GLdouble matrix_stack[MATRIX_STACK_DEPTH][4][4] = {};
static int matrix_stack_current = 0;

static void glMyPushMatrix(void)
{
	glGetDoublev(GL_MODELVIEW_MATRIX, &matrix_stack[matrix_stack_current][0][0]);
	matrix_stack_current++;
}

static void glMyPopMatrix(void)
{
	matrix_stack_current--;
	glLoadMatrixd(&matrix_stack[matrix_stack_current][0][0]);
}

static void draw_led(led_t *led)
{
	double start_width = LED_WIDTH + led->start;
	double end_width = LED_WIDTH + led->end;
	unsigned long flags = led->flags;

	glMyPushMatrix();

	glScaled(1.0 / LED_WIDTH, 1.0 / LED_WIDTH, 1.0 / LED_WIDTH);
	glScaled(1.0 / LED_WIDTH, 1.0 / LED_WIDTH, 1.0 / LED_WIDTH);

	glTranslated(led->shift, 0, 0);

	glBegin(GL_POLYGON);

	glVertex2f(0, -0.5);
	glVertex2f(start_width - 0.5, -0.5);

	if (flags & LED_SHARP_START) {
		if (flags & LED_SHARP_START_RIGHT)
			glVertex2f(start_width+0.5, 0.5);
		else
			glVertex2f(start_width+0.5, -0.5);
	}
	else
		glVertex2f(start_width, 0);

	glVertex2f(start_width - 0.5, 0.5);
	glVertex2f(0, 0.5);

	glVertex2f(-(end_width - 0.5), 0.5);

	if (flags & LED_SHARP_END) {
		if (flags & LED_SHARP_END_RIGHT)
			glVertex2f(-end_width - 0.5, -0.5);
		else
			glVertex2f(-end_width - 0.5, 0.5);
	}
	else
		glVertex2f(-end_width, 0);

	glVertex2f(-(end_width - 0.5), -0.5);

	glEnd();

	glMyPopMatrix();
}

#define DIGIT_LED_SPACING 1.2
#define DIGIT_WIDTH  0.83

typedef struct led_windex_s {
	int index;
	led_t info;
} led_windex_t;

static void draw_digit_led(int led_index, led_windex_t *leds)
{
	if (led_index == 0)
		glTranslated(0, 1 * DIGIT_LED_SPACING, 0);

	if (led_index == 6)
		glTranslated(0, -1 * DIGIT_LED_SPACING, 0);

	if (led_index == 1 ||
	    led_index == 2 ||
	    led_index == 4 ||
	    led_index == 5 ||
	    led_index == 7 ||
	    led_index == 8)
		{
			if (led_index == 1 ||
			    led_index == 7 ||
			    led_index == 2)
				{
					glTranslated(0, 0.5 * DIGIT_LED_SPACING, 0);
				}
			else if (led_index == 4 ||
				 led_index == 8 ||
				 led_index == 5)
				{
					glTranslated(0, -0.5 * DIGIT_LED_SPACING, 0);
				}

			if (led_index == 1 ||
			    led_index == 4)
				{
					glTranslated(-0.5 * DIGIT_LED_SPACING, 0, 0);
				}
			else if (led_index == 2 || led_index == 5)
				{
					glTranslated(0.5 * DIGIT_LED_SPACING, 0, 0);
				}

			glRotated(90, 0, 0, 1);
		}

	draw_led(&leds->info);
}

static void draw_digit(int nleds, led_windex_t *leds)
{
	int i;
	double ratio = 1.0 / (2 * 1*DIGIT_LED_SPACING);

	glMyPushMatrix();

	glScaled(ratio, ratio, ratio);

	for (i = 0; i < nleds; i++) {
		glMyPushMatrix();
		draw_digit_led(leds->index, leds);
		glMyPopMatrix();

		leds++;
	}

	glMyPopMatrix();
}

typedef struct digit_led_info_s {
	double total_offset;
	int num_leds;
	led_windex_t leds[10];
} digit_led_info_t;

static digit_led_info_t digit_led_info[10] = {
	[0] = {
		.num_leds = 6,
		.total_offset = 0.01,
		.leds = {
			{ .index = 0 },
			{ .index = 1, .info = { .end = 0.15 }},
			{ .index = 2, .info = { .end = 0.15 }},
			{ .index = 4, .info = { .start = 0.15 }},
			{ .index = 5, .info = { .start = 0.15 }},
			{ .index = 6 },
		},
	},
	[1] = {
		.num_leds = 3,
		.total_offset = 0.012,
		.leds = {
			{ .index = 0, .info = { .start = -1.0, .end = -1.0, .shift = 1.0,
						.flags = LED_SHARP_START | LED_SHARP_START_RIGHT }},
			{ .index = 2, .info = { .end = 0.15 ,
						.flags = LED_SHARP_START | LED_SHARP_START_LEFT}},
			{ .index = 5, .info = { .start = 0.15, .end = 0.5 }},
		},
	},
	[2] = {
		.num_leds = 5,
		.total_offset = 0.008,
		.leds = {
			{ .index = 0 },
			{ .index = 2 },
			{ .index = 3 },
			{ .index = 4, .info = { .flags = LED_SHARP_END | LED_SHARP_END_LEFT }},
			{ .index = 6, .info = { .flags = LED_SHARP_END | LED_SHARP_END_RIGHT }},
		},
	},
	[3] = {
		.num_leds = 5,
		.total_offset = 0.011,
		.leds = {
			{ .index = 0 },
			{ .index = 2 },
			{ .index = 3 },
			{ .index = 5 },
			{ .index = 6 },
		},
	},
	[4] = {
		.num_leds = 4,
		.total_offset = 0.012,
		.leds = {
			{ .index = 1, .info = { .start = 0.5 }},
			{ .index = 2, .info = { .start = 0.5 }},
			{ .index = 3 },
			{ .index = 5, .info = { .end = 0.5 }},
		},
	},
	[5] = {
		.num_leds = 5,
		.total_offset = 0.013,
		.leds = {
			{ .index = 0, .info = { .flags = LED_SHARP_END | LED_SHARP_START_RIGHT }},
			{ .index = 1, .info = { .flags = LED_SHARP_START | LED_SHARP_START_RIGHT }},
			{ .index = 3 },
			{ .index = 5 },
			{ .index = 6 },
		},
	},
	[6] = {
		.num_leds = 6,
		.total_offset = 0.010,
		.leds = {
			{ .index = 0, .info = { .flags = LED_SHARP_END | LED_SHARP_START_RIGHT }},
			{ .index = 1, .info = { .flags = LED_SHARP_START | LED_SHARP_START_RIGHT }},
			{ .index = 3 },
			{ .index = 5 },
			{ .index = 4 },
			{ .index = 6 },
		},
	},
	[7] = {
		.num_leds = 3,
		.total_offset = 0.009,
		.leds = {
			{ .index = 0, .info = { .flags = LED_SHARP_START | LED_SHARP_START_RIGHT }},
			{ .index = 2, .info = { .end = 0.15 , .flags = LED_SHARP_START | LED_SHARP_START_LEFT}},
			{ .index = 5, .info = { .start = 0.15, .end = 0.5 }},
		},
	},
	[8] = {
		.num_leds = 7,
		.total_offset = 0.011,
		.leds = {
			{ .index = 0 },
			{ .index = 1 },
			{ .index = 2 },
			{ .index = 3 },
			{ .index = 4 },
			{ .index = 5 },
			{ .index = 6 },
		},
	},
	[9] = {
		.num_leds = 6,
		.total_offset = 0.012,
		.leds = {
			{ .index = 0 },
			{ .index = 1 },
			{ .index = 2 },
			{ .index = 3 },
			{ .index = 5 },
			{ .index = 6 },
		},
	},
};

static void draw_colon_square(void)
{
	glBegin(GL_POLYGON);
	glVertex2f(0, 0);
	glVertex2f(0, 0.12);
	glVertex2f(0.12, 0.12);
	glVertex2f(0.12, 0);
	glEnd();
}

static void draw_colon(void)
{
	glMyPushMatrix();

	glTranslated(0, -0.2, 0);

	draw_colon_square();

	glTranslated(0, -0.36, 0);

	draw_colon_square();

	glMyPopMatrix();
}

static void draw_specific_digit(int digit)
{
	digit_led_info_t *inf = NULL;

	glMyPushMatrix();

	inf = &digit_led_info[digit];

	glTranslated(inf->total_offset * 4, 0, 0);

	draw_digit(inf->num_leds, &inf->leds[0]);

	glMyPopMatrix();
}

typedef struct time24 {
	int hours;
	int seconds;
	int minutes;
} time24_t;

static void decode_time(time_t time, time24_t *t24)
{
	struct tm tm;

	tm = *localtime(&time);
	t24->hours = tm.tm_hour % 24;
	t24->minutes = tm.tm_min;
	t24->seconds = tm.tm_sec;
}

static void draw_clock(time24_t *t24)
{
	glMyPushMatrix();

	draw_specific_digit(t24->hours / 10);
	glTranslated(0.78, 0, 0);

	draw_specific_digit(t24->hours % 10);
	glTranslated(0.59, 0, 0);

	draw_colon();
	glTranslated(0.55, 0, 0);

	draw_specific_digit(t24->minutes / 10);
	glTranslated(0.78, 0, 0);

	draw_specific_digit(t24->minutes % 10);
	glTranslated(0.59, 0, 0);

	draw_colon();
	glTranslated(0.55, 0, 0);

	draw_specific_digit(t24->seconds / 10);
	glTranslated(0.78, 0, 0);

	draw_specific_digit(t24->seconds % 10);

	glMyPopMatrix();
}

void scene_display(time_t time)
{
	GLdouble matrix[4][4] = {};
	time24_t t24;

	glMatrixMode (GL_MODELVIEW);

	glMyPushMatrix();

	matrix[0][0] = 1;
	matrix[1][1] = 1;
	matrix[1][0] = 0.14;
	matrix[2][2] = 1;
	matrix[3][3] = 1;

	decode_time(time, &t24);

	glMultMatrixd(&matrix[0][0]);

	glTranslated(-0.7, 0, 0);

	glScaled(0.3, 0.3, 0.3);

	glColor3d(160.0 / 377, 148.0 / 377, 30.0 / 377);

	glMyPushMatrix();
	glTranslated(-0.01, 0, 0);
	draw_clock(&t24);
	glMyPopMatrix();

	glMyPushMatrix();
	glTranslated(0.01, 0, 0);
	draw_clock(&t24);
	glMyPopMatrix();

	glMyPushMatrix();
	glTranslated(0, -0.01, 0);
	draw_clock(&t24);
	glMyPopMatrix();

	glMyPushMatrix();
	glTranslated(0, 0.01, 0);
	draw_clock(&t24);
	glMyPopMatrix();

	glColor3d(160.0 / 255, 148.0 / 255, 30.0 / 255);

	draw_clock(&t24);

	glMyPopMatrix();
}

void scene_reshape(int w, int h)
{
	if (h < w)
		glViewport((w - h) / 2, 0, h, h);
	else
		glViewport(0, (h - w) / 2, w, w);

	glClearColor (0.0, 0.0, 0.0, 1.0);

	glColor3f(1.0, 1.0, 1.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
}

void scene_init(int w, int h)
{
	glClearColor(0.4, 0.4, 0.4, 0.0);
	glClearDepth(1.0);

	scene_reshape(w, h);
}
