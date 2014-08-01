#ifndef __24CLOCK_SCENE__
#define __24CLOCK_SCENE__

#include <sys/time.h>

void scene_display(time_t time);
void scene_reshape(int w, int h);
void scene_init(int w, int h);

#endif
