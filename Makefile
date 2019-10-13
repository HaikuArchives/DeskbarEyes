CFLAGS=-O3
LIBS=-lroot -lbe -ltranslation
CC=g++
OBJS = Eyes.o EyesView.o

all: build

build: $(OBJS)
	$(CC) $(CFLAGS) -o DeskbarEyes $^ $(LIBS)
	rc Resource.rdef
	xres -o DeskbarEyes Resource.rsrc
	mimeset -f DeskbarEyes

.cpp.o:
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f DeskbarEyes $(OBJS)
