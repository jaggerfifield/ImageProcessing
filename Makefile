
CC := gcc
FLAGS := -Wall
TARGET := ImageProcessor

DEP := png.o

all: $(DEP)
	$(CC) -Wall -ggdb png.o -o $(TARGET)

png.o: png.c
	$(CC) $(FLAGS) -c png.c

clean:
	rm -f $(TARGET)

