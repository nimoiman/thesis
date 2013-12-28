CC=gcc
CFLAGS =-I.
DEPS=vq.h channel.h
OBJ=vector_main.o vq.o channel.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

vector_main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS):w

