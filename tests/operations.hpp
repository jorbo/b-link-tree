#ifndef TESTS_OPERATIONS_HPP
#define TESTS_OPERATIONS_HPP


extern "C" {
#include "../operations.h"
};


TEST(OperationsTest, InsertRandom) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	constexpr uint_fast8_t OP_COUNT = 32;
	Request reqs[OP_COUNT];
	Response resps[OP_COUNT];
	uint_fast8_t i = 0;

	memset(reqs, 0, sizeof(Request)*OP_COUNT);
	memset(resps, 0, sizeof(Response)*OP_COUNT);
	mem_reset_all(memory);
	srand(0);
	bptr_t root = bptr_make(0, 0);

	// Generate Requests
	for (; i < OP_COUNT/2; ++i) {
		reqs[i].opcode = INSERT;
		reqs[i].insert.key = rand() % (1UL << ((8*sizeof(bkey_t))-2));
		reqs[i].insert.value.data = rand() % (1UL << (8*sizeof(bdata_t)));
	}
	for (; i < OP_COUNT; ++i) {
		reqs[i].opcode = SEARCH;
		reqs[i].search = reqs[i-(OP_COUNT/2)].insert.key;
	}
	// Execute requests and store responses
	for (i = 0; i < OP_COUNT; ++i) {
		resps[i] = execute_req(reqs[i], &root, memory);
		EXPECT_EQ(resps[i].opcode, i < OP_COUNT/2 ? INSERT : SEARCH);
		EXPECT_EQ(i < (OP_COUNT/2) ? resps[i].search.status : resps[i].insert, SUCCESS);
	}
	
	fprintf(log_stream, "\n\n");
}


#endif
