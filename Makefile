CC=gcc
CFLAGS=-I. -O2 -Wall -Wextra -Wpedantic -std=gnu99
LDFLAGS=-lm

vq_DIR=vq/
covq_DIR=covq/
covq_2_DIR=covq_2/

vq_OBJS=vq_main.o vq.o vector.o channel.o
covq_OBJS=covq_main.o covq.o channel.o vector.o anneal.o io.o
test_OBJS=test.o covq.o channel.o vector.o anneal.o io.o
covq_2_OBJS=covq_main.o covq.o io.o quantize.o anneal.o running.o channel.o
test_2_OBJS=test.o covq.o io.o quantize.o anneal.o running.o

all: vq covq covq_2

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

vq: $(addprefix $(vq_DIR),$(vq_OBJS))
	$(CC) -o $(addprefix $(vq_DIR),$@) $^ $(LDFLAGS) $(CFLAGS)

covq: $(addprefix $(covq_DIR),$(covq_OBJS))
	$(CC) -o $(addprefix $(covq_DIR),$@) $^ $(LDFLAGS) $(CFLAGS)

test: $(addprefix $(covq_DIR),$(test_OBJS))
	$(CC) -o $(addprefix $(covq_DIR),$@) $^ $(LDFLAGS) $(CFLAGS)

covq_2: $(addprefix $(covq_2_DIR),$(covq_2_OBJS))
	$(CC) -o $(addprefix $(covq_2_DIR),$@) $^ $(LDFLAGS) $(CFLAGS)

test_2: $(addprefix $(covq_2_DIR),$(test_2_OBJS))
	$(CC) -o $(addprefix $(covq_2_DIR),$@) $^ $(LDFLAGS) $(CFLAGS)

.PHONY : clean
clean:
	rm -f $(addprefix $(vq_DIR),$(vq_OBJS)) \
	   $(addprefix $(vq_DIR),vq) \
	   $(addprefix $(covq_DIR),$(covq_OBJS)) \
	   $(addprefix $(covq_DIR),covq) \
	   $(addprefix $(covq_DIR),$(test_OBJS)) \
	   $(addprefix $(covq_DIR),test) \
	   $(addprefix $(covq_2_DIR),$(covq_2_OBJS)) \
	   $(addprefix $(covq_2_DIR),covq_2) \
	   $(addprefix $(covq_2_DIR),$(test_2_OBJS)) \
	   $(addprefix $(covq_2_DIR),test)