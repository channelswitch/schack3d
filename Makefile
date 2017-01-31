a.out: GL_utilities.o MicroGlut.o schack.o VectorUtils3.o loadobj.o LoadTGA.o
	cc -g -std=c99 `pkg-config --cflags --libs sdl gl x11` -lm *.o 

MicroGlut.o: MicroGlut.c MicroGlut.h
	cc -c -g -std=c99 `pkg-config --cflags --libs sdl gl` -DGL_GLEXT_PROTOTYPES -o MicroGlut.o MicroGlut.c

VectorUtils3.o: VectorUtils3.c VectorUtils3.h
	cc -c -g -std=c99 `pkg-config --cflags --libs sdl gl` -DGL_GLEXT_PROTOTYPES -o VectorUtils3.o VectorUtils3.c

GL_utilities.o: GL_utilities.c GL_utilities.h
	cc -c -g -std=c99 `pkg-config --cflags --libs sdl gl` -DGL_GLEXT_PROTOTYPES -o GL_utilities.o GL_utilities.c

loadobj.o: loadobj.c loadobj.h
	cc -c -g -std=c99 `pkg-config --cflags --libs sdl gl` -DGL_GLEXT_PROTOTYPES -o loadobj.o loadobj.c

LoadTGA.o: LoadTGA.c LoadTGA.h
	cc -c -g -std=c99 `pkg-config --cflags --libs sdl gl` -DGL_GLEXT_PROTOTYPES -o LoadTGA.o LoadTGA.c

schack.o: schack.c
	cc -c -g -std=c99 `pkg-config --cflags --libs sdl gl` -DGL_GLEXT_PROTOTYPES -o schack.o schack.c
