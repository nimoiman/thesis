CC=gcc
CFLAGS =-I.
LDFLAGS=-lm
DEPS=vq.h channel.h
OBJ=vector_main.o vq.o channel.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

vector_main: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS):w

clean:
	rm vector_main $(OBJ)