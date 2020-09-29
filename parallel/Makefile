SRC_C = parallel.c point.c octree.c compress.c fileIO.c queue.c memory.c diff.c test.c merge.c
SRC_H = octree.h queue.h

all: debug

debug: $(SRC_C) $(SRC_H)
	gcc -g $(SRC_C) $(SRC_H) -o parallel
	
opt: $(SRC_C) $(SRC_H)
	gcc -O3 $(SRC_C) $(SRC_H) -o parallel

%.o: %.c
	gcc -c $<

clean:
	rm parallel
	
run:
	./parallel 8 1000 ../output/test.pcs ../input/clouds/

val:
	valgrind ./parallel 8 1000 ../output/test.pcs ../input/clouds/

gdb:
	gdb --args ./parallel 8 1000 ../output/test.pcs ../input/clouds/
