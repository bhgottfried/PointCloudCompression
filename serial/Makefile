all: single.o compress.o octree.o queue.o point.o
	gcc -o serial single.o compress.o octree.o queue.o point.o

%.o: %.c
	gcc -c $<

clean:
	rm serial *.o