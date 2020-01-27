SRC_C = serial.c point.c octree.c compress.c fileIO.c queue.c
SRC_H = octree.h queue.h

all: serial.o compress.o octree.o queue.o point.o
	gcc -o serial serial.o compress.o octree.o queue.o point.o

debug:
	gcc -g $(SRC_C) $(SRC_H) -o serial

%.o: %.c
	gcc -c $<

clean:
	rm serial *.o
