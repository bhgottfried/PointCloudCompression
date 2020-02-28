SRC_C = serial.c point.c octree.c compress.c fileIO.c queue.c memory.c diff.c test.c merge.c
SRC_H = octree.h queue.h

all: serial.o point.o octree.o compress.o fileIO.o queue.o memory.o diff.o test.o merge.o
	gcc -o serial serial.o point.o octree.o compress.o fileIO.o queue.o memory.o diff.o test.o merge.o

debug: $(SRC_C) $(SRC_H)
	gcc -g $(SRC_C) $(SRC_H) -o serial

%.o: %.c
	gcc -c $<

clean:
	rm serial *.o
