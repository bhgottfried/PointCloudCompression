SRC_C = serial.c point.c octree.c compress.c fileIO.c queue.c memory.c diff.c test.c merge.c
SRC_H = octree.h queue.h

default: $(SRC_C) $(SRC_H)
	gcc -g $(SRC_C) $(SRC_H) -o serial
	
opt: $(SRC_C) $(SRC_H)
	gcc -O3 $(SRC_C) $(SRC_H) -o serial

%.o: %.c
	gcc -c $<

clean:
	rm serial
	
run:
	./serial -s 5 5 ../output/test.pcs ../input/rand2.pcd ../input/rand5.pcd ../input/rand3.pcd ../input/rand1000.pcd ../input/rand10.shift.pcd

val:
	valgrind ./serial -s 5 5 ../output/test.pcs ../input/rand2.pcd ../input/rand5.pcd ../input/rand3.pcd ../input/rand1000.pcd ../input/rand10.shift.pcd

gdb:
	gdb --args ./serial -s 5 5 ../output/test.pcs ../input/rand2.pcd ../input/rand5.pcd ../input/rand3.pcd ../input/rand1000.pcd ../input/rand10.shift.pcd
