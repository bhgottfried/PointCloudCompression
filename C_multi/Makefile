EXECUTABLE = PtCloudMulti
SRC_C = multi.c
SRC_H = multi.h miniunit.h
HELP_C = basics.c
TEST_C = test.c
CC = /opt/gcc/7.1.0/bin/gcc -std=c11
CFLAGS = -g -Werror -Wall -Wvla -pedantic
CFLAGS_GCOV = $(CFLAGS) -fprofile-arcs -ftest-coverage

$(EXECUTABLE): $(TEST_C) $(SRC_C) $(SRC_H) $(HELP_C)
	@$(CC) -o $(EXECUTABLE) $(TEST_C) $(SRC_C) $(HELP_C) $(CFLAGS)

coverage: $(SRC_C) $(SRC_H)
	@$(CC) -o $(EXECUTABLE) $(SRC_C) $(TEST_C) $(CFLAGS_GCOV)
	@./$(EXECUTABLE) > $(TEST_ACTUAL)
	@gcov -f $(SRC_C)

val: $(EXECUTABLE)
	@valgrind ./$(EXECUTABLE)

valv: $(EXECUTABLE)
	@valgrind -v ./$(EXECUTABLE)
	
clean:
	@rm -f $(EXECUTABLE) core.* *.gcno *.gcda *.gcov
