CC=gcc
PROG=vector
CFLAGS=-I.
LDFLAGS=-lm
OBJ=vq_main.o vq.o channel.o vector.o


$(PROG): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

covq: covq_main.o covq.o channel.o vector.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

clean:
	rm $(PROG) $(OBJ)

