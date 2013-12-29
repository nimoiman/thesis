CC=gcc
PROG=vector
CFLAGS=-I.
LDFLAGS=-lm
OBJ=vq_main.o vq.o channel.o vector.o


$(PROG): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm $(PROG) $(OBJ)

