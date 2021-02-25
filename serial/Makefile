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
	./serial -s 4 4 ../output/serial.pcs ../input/clouds/ #| grep time

val:
	valgrind --track-origins=yes --leak-check=full ./serial -s 2 2 ../output/test.pcs ../input/clouds/

gdb:
	gdb --args ./serial -s 4 6 ../output/test.pcs ../input/clouds/
