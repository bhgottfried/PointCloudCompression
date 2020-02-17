SRC_C = serial.c point.c octree.c compress.c fileIO.c queue.c
SRC_H = octree.h queue.h

all: serial.o point.o octree.o compress.o fileIO.o queue.o
	gcc -o serial serial.o point.o octree.o compress.o fileIO.o queue.o

debug: $(SRC_C) $(SRC_H)
	gcc -g $(SRC_C) $(SRC_H) -o serial

%.o: %.c
	gcc -c $<

clean:
	rm serial *.o
