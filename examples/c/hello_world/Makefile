SRC_FILES = hello_world.c
LIBPD = ../../../libs/libpd.so
TARGET = hello_world
DIST_DIR = ../bin

CFLAGS = -I../../../../pure-data/src -I../../../libpd_wrapper -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.c=.o} $(LIBPD)
	gcc -Wall -O2 -pedantic -o $(DIST_DIR)/$(TARGET) $^ 

$(LIBPD):
	cd ../../../.. && make

clean:
	rm -f *.o

clobber: clean
	rm -f $(TARGET)
