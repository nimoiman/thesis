CC=gcc
PROG=vector
CFLAGS=-I.
LDFLAGS=-lm
OBJ=vector_main.o vq.o

$(PROG): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm $(PROG) $(OBJ)
