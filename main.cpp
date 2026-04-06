#include "defs.h"
extern "C" {
#include "node.h"
};
#ifndef NO_GTEST
#include "tests/misc.hpp"
#include "tests/search.hpp"
#include "tests/insert.hpp"
#include "tests/parallel.hpp"
#include "tests/operations.hpp"
#endif
#include "thread-runner.hpp"
#include <iostream>
#include <cstring>
#include <cstdio>


FILE *log_stream = fopen("main.log", "w");
static Node mem_data[MAX_LEVELS][MAX_NODES_PER_LEVEL];
Node *memory[MAX_LEVELS];


#ifndef NO_GTEST
static int run_gtests(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	int status = RUN_ALL_TESTS();
	fclose(log_stream);
	return status;
}
#endif


int main(int argc, char **argv) {
	for (int i = 0; i < MAX_LEVELS; i++) memory[i] = mem_data[i];
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
