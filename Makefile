# Makefile
# Patrick MacArthur <generalpenguin89@gmail.com>

CFLAGS=-g -Wall -Wextra -O

OBJECTS=tpool.o queue.o future.o

.PHONY: all
all: $(OBJECTS)

tpool.o: tpool.h tpool_private.h

queue.o: tpool.h tpool_private.h

future.o: tpool.h tpool_private.h

.PHONY: clean
clean:
	$(RM) *.o
