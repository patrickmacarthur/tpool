# Makefile
# Patrick MacArthur <generalpenguin89@gmail.com>

CFLAGS=-g -Wall -Wextra -O -fPIC

OBJECTS=tpool.o queue.o future.o

.PHONY: all
all: libtpool.so test tags

libtpool.so: $(OBJECTS)
	$(CC) -shared $(LDFLAGS) -o $@ $^

tpool.o: tpool.h tpool_private.h

queue.o: tpool.h tpool_private.h

future.o: tpool.h tpool_private.h

LIBS=-lpthread -L. -ltpool
test: test.o tpool.h
	$(CC) $(LIBS) -o $@ $^

.PHONY: clean
clean:
	$(RM) test libtpool.so *.o

.PHONY: cleanall
cleanall: clean
	$(RM) tags

tags:
	ctags *.c *.h
