// vim: fileencoding=utf8 expandtab tabstop=4 shiftwidth=4
#ifndef __MINIUNIT_H__
#define __MINIUNIT_H__
	#include <stdio.h>
	#include <stdbool.h>
	#include <unistd.h>

	const char* ANSI_RED = "\x1b[31m";
	const char* ANSI_GREEN = "\x1b[32m";
	const char* ANSI_YELLOW = "\x1b[33m";
	const char* ANSI_BLUE = "\x1b[34m";
	const char* ANSI_MAGENTA = "\x1b[35m";
	const char* ANSI_CYAN = "\x1b[36m";
	const char* ANSI_RESET = "\x1b[0m";

	#define mu_start() int __muLineNumberFailed = 0;

	#define mu_check(condition) do {															\
			if (!(condition) && !__muLineNumberFailed) {	/* if the test condition fails and
														  	it is the first occurence of
														  	failure (line # is still 0) */		\
				__muLineNumberFailed = __LINE__;												\
			}																					\
		} while (false)

	#define mu_run(function) do {																\
		int __muErrorLine = function();															\
		if (__muErrorLine) {		/* An error occured in mu_check so the line # =/= 0 */		\
			if (isatty(STDOUT_FILENO)) {														\
				fputs(ANSI_RED, stdout);														\
				fprintf(stdout, "Test failed: (%s) at line %d\n", (#function), __muErrorLine);	\
				fputs(ANSI_RESET, stdout);														\
			}																					\
			else {																				\
				fprintf(stdout, "Test failed: (%s) at line %d\n", (#function), __muErrorLine);	\
			}																					\
		}																						\
		else {		/* mu_check passed so the line number will still be 0 */					\
			if (isatty(STDOUT_FILENO)) {														\
				fputs(ANSI_GREEN, stdout);														\
				fprintf(stdout, "Test passed: (%s)\n", (#function));							\
				fputs(ANSI_RESET, stdout);														\
			}																					\
			else {																				\
				fprintf(stdout, "Test passed: (%s)\n", (#function));							\
			}																					\
		}																						\
	} while(false)

	#define mu_end() return __muLineNumberFailed;
#endif
