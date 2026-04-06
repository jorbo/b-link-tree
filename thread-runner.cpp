#include "loader.hpp"
#include "thread-runner.hpp"
extern "C" {
#include "memory.h"
#include "node.h"
#include "operations.h"
};
#include <iostream>
#include <stdint.h>


extern Node *memory[MAX_LEVELS];


struct ThreadArgs {
	uint_fast8_t thread_id;
	uint_fast8_t thread_count;
	uint_fast32_t op_count;
	Request* requests;
	Response* responses;
	bptr_t* root;
};

static void *tree_thread(void *argv) {
	ThreadArgs* args = (ThreadArgs *)argv;
#ifdef STRIDED
	for (uint_fast64_t i = args->thread_id; i < args->op_count; i += args->thread_count) {
#else // BLOCKED
	const uint_fast64_t BLOCK_SIZE = args->op_count/args->thread_count;
	for (uint_fast64_t i = args->thread_id * BLOCK_SIZE; i < (args->thread_id+1)*BLOCK_SIZE; ++i) {
#endif
		args->responses[i] = execute_req(args->requests[i], args->root, memory);
	}
	pthread_exit(NULL);
}


int run_from_file(int argc, char **argv) {
	// Thread stuff
	uint_fast8_t offset = 0;
	uint_fast8_t last = 0;
	std::vector<uint_fast8_t> then_splits;
	std::vector<pthread_t> threads(argc-2);
	std::vector<ThreadArgs> threads_args(argc-2);
	// Tree data
	std::vector<std::vector<Request>> reqbufs(argc-2);
	std::vector<std::vector<Response>> respbufs(argc-2);
	bptr_t root = 0;
	// Timing
	clock_t timer;

	if (argc < 3) {
		std::cerr << "Missing request file" << std::endl;
		return 1;
	}

	for (uint_fast8_t i = 0; i < argc-2; ++i) {
		if (std::string(argv[i+2]) == "then") {
			then_splits.push_back(i);
			offset++;
		} else {
			if (read_req_file(argv[i+2], reqbufs.at(i-offset))) {
				std::cerr << "Failed to read file " << argv[i+2] << std::endl;
				return 1;
			}
			respbufs.at(i-offset).resize(reqbufs.at(i-offset).size());
			threads_args.at(i-offset).thread_id = i-offset;
			threads_args.at(i-offset).op_count = reqbufs.at(i-offset).size();
			threads_args.at(i-offset).requests = reqbufs.at(i-offset).data();
			threads_args.at(i-offset).responses = respbufs.at(i-offset).data();
			threads_args.at(i-offset).root = &root;
		}
	}
	then_splits.push_back(argc-2-offset);
	last = 0;
	for (uint_fast8_t i = 0; i < then_splits.size(); ++i) {
		const uint_fast8_t sub_count = then_splits.at(i) - last;
		for (uint_fast8_t j = last; j < then_splits.at(i); ++j) {
			threads_args.at(j).thread_id -= last;
			threads_args.at(j).thread_count = sub_count;
		}
		last += sub_count;
	}

	// Execute requests
	mem_reset_all(memory);
	last = 0;
	for (uint_fast8_t split : then_splits) {
		std::cout << "Executing on " << (int) threads_args.at(last).thread_count << " threads..." << std::flush;
		timer = clock();
		for (uint_fast8_t i = last; i < split; ++i) {
			pthread_create(&threads.at(i), NULL, tree_thread, (void*) &threads_args.at(i));
		}
		for (uint_fast8_t i = last; i < split; ++i) {
			pthread_join(threads.at(i), NULL);
		}
		timer = clock() - timer;
		last = split;
		std::cout << "\ncompleted in " << (1000.0d * timer/CLOCKS_PER_SEC) << "ms" << std::endl;
	}

	return 0;
}
