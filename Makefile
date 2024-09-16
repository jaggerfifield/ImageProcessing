
CC := cc
TARGET := ImageProcessor.exe

DEP := png.o

all: $(DEP)
	$(CC) /Ze png.obj -o$(TARGET)

png.o: png.c
	$(CC) /Ze -c png.c

clean:
	rm -f $(TARGET)

