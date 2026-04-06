#ifndef TESTS_INSERT_HPP
#define TESTS_INSERT_HPP


extern "C" {
#include "../insert.h"
#include "../io.h"
#include "../memory.h"
#include "../node.h"
#include "../validate.h"
};
#include "helpers.hpp"
#include <gtest/gtest.h>
#include <pthread.h>

extern FILE *log_stream;
extern Node *memory[MAX_LEVELS];


TEST(InsertTest, LeafNode) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	bptr_t root = bptr_make(0, 0);
	mem_reset_all(memory);
	bval_t value;

	value.data = 2;
	EXPECT_EQ(insert(&root, 0, value, memory), SUCCESS);
	EXPECT_EQ(mem_read(root, memory).keys[0], 0);
	EXPECT_EQ(mem_read(root, memory).values[0].data, 2);
	dump_node_list(log_stream, memory);
	ASSERT_TRUE(is_unlocked(root, log_stream, memory));

	value.data = 3;
	EXPECT_EQ(insert(&root, 5, value, memory), SUCCESS);
	EXPECT_EQ(mem_read(root, memory).keys[1], 5);
	EXPECT_EQ(mem_read(root, memory).values[1].data, 3);
	dump_node_list(log_stream, memory);
	ASSERT_TRUE(is_unlocked(root, log_stream, memory));

	value.data = 1;
	EXPECT_EQ(insert(&root, 3, value, memory), SUCCESS);
	EXPECT_EQ(mem_read(root, memory).keys[1], 3);
	EXPECT_EQ(mem_read(root, memory).values[1].data, 1);
	EXPECT_EQ(mem_read(root, memory).keys[2], 5);
	EXPECT_EQ(mem_read(root, memory).values[2].data, 3);
	dump_node_list(log_stream, memory);
	ASSERT_TRUE(is_unlocked(root, log_stream, memory));

	EXPECT_TRUE(validate(root, log_stream, memory));
	fprintf(log_stream, "\n\n");
}

TEST(InsertTest, SplitRoot) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	bptr_t root = bptr_make(0, 0);
	bptr_t lchild;
	bval_t value;
	mem_reset_all(memory);

	value.data = 0;
	EXPECT_EQ(insert(&root, -value.data, value, memory), SUCCESS);
	dump_node_list(log_stream, memory);
	EXPECT_EQ(mem_read(root, memory).keys[0], -value.data);
	EXPECT_EQ(mem_read(root, memory).values[0].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream, memory));
	value.data = -5;
	EXPECT_EQ(insert(&root, -value.data, value, memory), SUCCESS);
	dump_node_list(log_stream, memory);
	EXPECT_EQ(mem_read(root, memory).keys[1], -value.data);
	EXPECT_EQ(mem_read(root, memory).values[1].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream, memory));
	value.data = -3;
	EXPECT_EQ(insert(&root, -value.data, value, memory), SUCCESS);
	dump_node_list(log_stream, memory);
	EXPECT_EQ(mem_read(root, memory).keys[1], -value.data);
	EXPECT_EQ(mem_read(root, memory).values[1].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream, memory));
	// This one causes a split
	value.data = -1;
	EXPECT_EQ(insert(&root, -value.data, value, memory), SUCCESS);
	dump_node_list(log_stream, memory);
	lchild = mem_read(root, memory).values[0].ptr;
	EXPECT_EQ(mem_read(lchild, memory).keys[1], -value.data);
	EXPECT_EQ(mem_read(lchild, memory).values[1].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream, memory));
	value.data = -4;
	EXPECT_EQ(insert(&root, -value.data, value, memory), SUCCESS);
	dump_node_list(log_stream, memory);
	ASSERT_TRUE(is_unlocked(root, log_stream, memory));
	ASSERT_TRUE(is_unlocked(lchild, log_stream, memory));

	EXPECT_TRUE(validate(root, log_stream, memory));
	fprintf(log_stream, "\n\n");
}

TEST(InsertTest, SequentialInsert) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	bptr_t root = bptr_make(0, 0);
	bval_t value;
	mem_reset_all(memory);

	// Insert values
	for (uint_fast8_t i = 1; i <= (TREE_ORDER/2)*(MAX_LEAVES+1); ++i) {
		value.data = -i;
		ASSERT_EQ(insert(&root, i, value, memory), SUCCESS);
		dump_node_list(log_stream, memory);
		ASSERT_TRUE(is_unlocked(root, log_stream, memory));
	}
	EXPECT_TRUE(check_inserted_leaves());

	EXPECT_TRUE(validate(root, log_stream, memory));
	fprintf(log_stream, "\n\n");
}


#endif
