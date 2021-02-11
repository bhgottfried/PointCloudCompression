SRC_C = parallel.c point.c octree.c compress.c fileIO.c queue.c memory.c diff.c test.c merge.c
SRC_H = octree.h queue.h

all: debug

debug: $(SRC_C) $(SRC_H)
	gcc -g -fopenmp $(SRC_C) $(SRC_H) -o parallel
	
opt: $(SRC_C) $(SRC_H)
	gcc -O2 -fopenmp $(SRC_C) $(SRC_H) -o parallel

%.o: %.c
	gcc -c $<

clean:
	rm parallel
	
run:
	./parallel 8 193 ../output/parallel.pcs ../input/clouds/ #| grep time

val:
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./parallel 4 9 ../output/test.pcs ../input/clouds/

gdb:
	gdb --args ./parallel 8 9 ../output/test.pcs ../input/clouds/
