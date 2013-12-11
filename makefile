CC=gcc
CFLAGS =-I.
DEPS=vq.h
OBJ=vector_main.o vq.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

vector_main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS):w

