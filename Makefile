CC=gcc
CFLAGS=-I. -O2 -Wall -Wextra -Wpedantic -std=gnu99
LDFLAGS=-lm

vq_DIR=vq/
covq_DIR=covq/
covq_2_DIR=covq_2/

vq_OBJS=vq_main.o vq.o vector.o channel.o
covq_OBJS=covq_main.o covq.o channel.o vector.o anneal.o io.o run.o
covq_2_OBJS=covq_main.o anneal.o init.o covq.o util.o run.o quantize.o
test_OBJS=test.o covq.o io.o quantize.o anneal.o running.o
covq2_test_OBJS=quantize_test.o quantize.o

all: vq covq covq_2

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

vq: $(addprefix $(vq_DIR),$(vq_OBJS))
	$(CC) -o $(addprefix $(vq_DIR),$@) $^ $(LDFLAGS) $(CFLAGS)

covq: $(addprefix $(covq_DIR),$(covq_OBJS))
	$(CC) -o $(addprefix $(covq_DIR),$@) $^ $(LDFLAGS) $(CFLAGS)

covq2_test: $(addprefix $(covq_2_DIR),$(covq2_test_OBJS))
	$(CC) -o $(addprefix $(covq_2_DIR),$@) $^ $(LDFLAGS) $(CFLAGS)

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
	   $(addprefix $(covq_2_DIR),$(covq2_test_OBJS)) \
	   $(addprefix $(covq_2_DIR),covq_2) \
	   $(addprefix $(covq_2_DIR),$(test_2_OBJS)) \
	   $(addprefix $(covq_2_DIR),test)

