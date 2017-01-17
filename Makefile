LIBCONFUSE_L := $(shell pkg-config libconfuse --libs)
LIBCONFUSE_I := $(shell pkg-config libconfuse --cflags)

CFLAGS=-O2 -g -Wall -ffast-math -Wstrict-prototypes -Wmissing-prototypes
SRCS=main.c scene.c
OBJS=main.o scene.o
TARGET=24clock
INCLUDES=
LIBRARIES=-lGL -lGLU -lglut -L/usr/X11R6/lib -lXt -lX11 -lXaw -lXi -lm $(LIBCONFUSE_L)

.c.o :
	gcc $(CFLAGS) -c -o $*.o $(INCLUDES) $<

$(TARGET) : $(OBJS)
	gcc $(OBJS) $(LIBRARIES) -o $@

clean:
	rm -f $(TARGET) $(OBJS)

dep:
	echo -n > depend.rules
	for I in $(SRCS) ; do cpp -M $$I $(INCLUDES) >> depend.rules ; done

-include depend.rules
