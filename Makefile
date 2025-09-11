CC = gcc
CFLAGS = -Wall -Wvla -fsanitize=address,undefined
TARGETS = alloc 

all: $(TARGETS)

alloc: alloc.c
	$(CC) $(CFLAGS) -o alloc alloc.c 

clean:
	rm -f *.o $(TARGETS)