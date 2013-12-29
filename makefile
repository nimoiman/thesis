CC=gcc
PROG=vector
CFLAGS=-I.
LDFLAGS=-lm
OBJ=vector_main.o vq.o channel.o


$(PROG): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm $(PROG) $(OBJ)

