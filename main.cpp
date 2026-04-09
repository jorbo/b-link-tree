#include "defs.h"
#ifndef NO_GTEST
#include "tests/misc.hpp"
#include "tests/search.hpp"
#include "tests/insert.hpp"
#include "tests/parallel.hpp"
#include "tests/operations.hpp"
#include "tests/distributed.hpp"
#endif
#include "thread-runner.hpp"
#include <iostream>
#include <cstring>
#include <cstdio>


FILE *log_stream = fopen("main.log", "w");
Node memory[MEM_SIZE];
mem_context_t ctx;


#ifndef NO_GTEST
static int run_gtests(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	int status = RUN_ALL_TESTS();
	fclose(log_stream);
	return status;
}
#endif


int main(int argc, char **argv) {
	ctx = mem_context_local(0, memory);
#ifndef NO_GTEST
	if (argc < 2 || strcmp(argv[1], "gtest") == 0) {
		return run_gtests(argc, argv);
	} else
#endif
	if (strcmp(argv[1], "exe") == 0) {
		return run_from_file(argc, argv);
	} else {
		std::cerr << "Unrecognized option, \""
			<< argv[1] << "\", valid options are: gtest, exe" << std::endl;
		return 1;
	}
}
