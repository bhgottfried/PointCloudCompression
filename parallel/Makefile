SRC_C = parallel.c point.c octree.c compress.c fileIO.c queue.c memory.c diff.c test.c merge.c
SRC_H = octree.h queue.h

default: $(SRC_C) $(SRC_H)
	gcc -g $(SRC_C) $(SRC_H) -o parallel
	
opt: $(SRC_C) $(SRC_H)
	gcc -O3 $(SRC_C) $(SRC_H) -o parallel

%.o: %.c
	gcc -c $<

clean:
	rm parallel
	
run:
	./parallel -s 5 17 ../output/test.pcs ../input/rand2.pcd ../input/rand5.pcd ../input/rand3.pcd ../input/rand1000.pcd ../input/rand10.shift.pcd ../input/rand2.pcd ../input/rand5.pcd ../input/rand10.pcd ../input/rand2.pcd ../input/rand10.shift.pcd ../input/rand3.pcd ../input/rand10.shift.pcd ../input/rand5.pcd ../input/rand1000.pcd ../input/rand3.pcd ../input/rand5.pcd ../input/rand2.pcd

val:
	valgrind ./parallel -s 5 17 ../output/test.pcs ../input/rand2.pcd ../input/rand5.pcd ../input/rand3.pcd ../input/rand1000.pcd ../input/rand10.shift.pcd ../input/rand2.pcd ../input/rand5.pcd ../input/rand10.pcd ../input/rand2.pcd ../input/rand10.shift.pcd ../input/rand3.pcd ../input/rand10.shift.pcd ../input/rand5.pcd ../input/rand1000.pcd ../input/rand3.pcd ../input/rand5.pcd ../input/rand2.pcd

gdb:
	gdb --args ./parallel -s 5 17 ../output/test.pcs ../input/rand2.pcd ../input/rand5.pcd ../input/rand3.pcd ../input/rand1000.pcd ../input/rand10.shift.pcd ../input/rand2.pcd ../input/rand5.pcd ../input/rand10.pcd ../input/rand2.pcd ../input/rand10.shift.pcd ../input/rand3.pcd ../input/rand10.shift.pcd ../input/rand5.pcd ../input/rand1000.pcd ../input/rand3.pcd ../input/rand5.pcd ../input/rand2.pcd
