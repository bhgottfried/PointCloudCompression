all: serial.o compress.o octree.o queue.o point.o
	gcc -o serial serial.o compress.o octree.o queue.o point.o

%.o: %.c
	gcc -c $<

clean:
	rm serial *.o