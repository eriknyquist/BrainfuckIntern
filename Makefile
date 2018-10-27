OBJ= $(patsubst %.c,%.o,$(wildcard *.c))
PROGNAME=bfintern

CFLAGS := -Wall -O3
.PHONY: clean

%: %.c

all: $(PROGNAME)
debug: CFLAGS = -Wall -O0 -g3
debug: $(PROGNAME)

$(PROGNAME): $(OBJ)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o
	rm -f $(PROGNAME)
