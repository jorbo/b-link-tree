#ifndef TESTS_PARALLEL_HPP
#define TESTS_PARALLEL_HPP


extern "C" {
#include "../io.h"
#include "../memory.h"
#include "../node.h"
#include "../validate.h"
};
#include "helpers.hpp"
#include <gtest/gtest.h>

// Multithreading bugs are often dependent on random ways that threads can
// interleave. Testing multiple times gives more chances for these bugs to
// manifest themselves.
#ifndef PARALLEL_RERUNS
#define PARALLEL_RERUNS 1024
#endif

extern FILE *log_stream;
extern Node memory[MEM_SIZE];
extern mem_context_t ctx;
constexpr uint_fast32_t KEY_MAX = (TREE_ORDER/2)*(MAX_LEAVES+1);


TEST(ParallelTest, InterleavedAscending) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	char strbuf[64];
	pthread_t thread_even, thread_odd;
	bptr_t root = 0;
	si_args odd_args = {
		.start = 1,
		.end = KEY_MAX,
		.stride = 2,
		.root = &root
	};
	si_args even_args = odd_args;
	even_args.start = 2;
	system("mkdir -p thread-logs");

	for (uint_fast16_t i = 0; i < PARALLEL_RERUNS; ++i) {
		fprintf(log_stream, "Run %d\n", i+1);
		root = 0;
		mem_reset_all(&ctx);
		sprintf(strbuf, "thread-logs/int-asc_run_%04d_%s.log", i, "odd");
		odd_args.log_stream = fopen(strbuf, "w");
		sprintf(strbuf, "thread-logs/int-asc_run_%04d_%s.log", i, "even");
		even_args.log_stream = fopen(strbuf, "w");

		pthread_create(&thread_even, NULL, stride_insert, (void*) &even_args);
		pthread_create(&thread_odd, NULL, stride_insert, (void*) &odd_args);
		pthread_join(thread_even, NULL);
		pthread_join(thread_odd, NULL);
		fclose(odd_args.log_stream);
		fclose(even_args.log_stream);
		ASSERT_TRUE(even_args.pass);
		ASSERT_TRUE(odd_args.pass);

		dump_node_list(log_stream, memory);

		ASSERT_TRUE(check_inserted_leaves());

		ASSERT_TRUE(validate(root, log_stream, &ctx));
	}
	fprintf(log_stream, "\n\n");
}

TEST(ParallelTest, InterleavedDescending) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	char strbuf[64];
	pthread_t thread_even, thread_odd;
	bptr_t root = 0;
	si_args odd_args = {
		.end = 1,
		.stride = -2,
		.root = &root
	};
	si_args even_args = odd_args;
	if (KEY_MAX % 2 == 0) {
		even_args.start = KEY_MAX;
		odd_args.start = KEY_MAX-1;
	} else {
		odd_args.start = KEY_MAX;
		even_args.start = KEY_MAX-1;
	}
	system("mkdir -p thread-logs");

	for (uint_fast16_t i = 0; i < PARALLEL_RERUNS; ++i) {
		fprintf(log_stream, "Run %d\n", i+1);
		root = 0;
		mem_reset_all(&ctx);
		sprintf(strbuf, "thread-logs/int-des_run_%04d_%s.log", i, "odd");
		odd_args.log_stream = fopen(strbuf, "w");
		sprintf(strbuf, "thread-logs/int-des_run_%04d_%s.log", i, "even");
		even_args.log_stream = fopen(strbuf, "w");

		pthread_create(&thread_even, NULL, stride_insert, (void*) &even_args);
		pthread_create(&thread_odd, NULL, stride_insert, (void*) &odd_args);
		fclose(odd_args.log_stream);
		fclose(even_args.log_stream);
		pthread_join(thread_even, NULL);
		pthread_join(thread_odd, NULL);
		ASSERT_TRUE(even_args.pass);
		ASSERT_TRUE(odd_args.pass);

		dump_node_list(log_stream, memory);

		ASSERT_TRUE(check_inserted_leaves());

		ASSERT_TRUE(validate(root, log_stream, &ctx));
	}
	fprintf(log_stream, "\n\n");
}

TEST(ParallelTest, CrossfadeInsert) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	char strbuf[64];
	pthread_t thread_even, thread_odd;
	bptr_t root = 0;
	si_args odd_args = {
		.start = 1,
		.end = KEY_MAX,
		.stride = 2,
		.root = &root,
		.pass = true
	};
	si_args even_args = odd_args;
	even_args.start = odd_args.end;
	even_args.end = odd_args.start % 2 == 0 ? odd_args.start : (odd_args.start + 1);
	even_args.stride = -odd_args.stride;
	system("mkdir -p thread-logs");

	for (uint_fast16_t i = 0; i < PARALLEL_RERUNS; ++i) {
		fprintf(log_stream, "Run %d\n", i+1);
		root = 0;
		mem_reset_all(&ctx);
		sprintf(strbuf, "thread-logs/x-fade_run_%04d_%s.log", i, "odd");
		odd_args.log_stream = fopen(strbuf, "w");
		sprintf(strbuf, "thread-logs/x-fade_run_%04d_%s.log", i, "even");
		even_args.log_stream = fopen(strbuf, "w");

		pthread_create(&thread_even, NULL, stride_insert, (void*) &even_args);
		pthread_create(&thread_odd, NULL, stride_insert, (void*) &odd_args);
		pthread_join(thread_even, NULL);
		pthread_join(thread_odd, NULL);
		fclose(odd_args.log_stream);
		fclose(even_args.log_stream);
		ASSERT_TRUE(even_args.pass);
		ASSERT_TRUE(odd_args.pass);

		dump_node_list(log_stream, memory);

		ASSERT_TRUE(check_inserted_leaves());

		ASSERT_TRUE(validate(root, log_stream, &ctx));
	}
	fprintf(log_stream, "\n\n");
}


#endif
