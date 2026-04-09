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
extern Node memory[MEM_SIZE];
extern mem_context_t ctx;


TEST(InsertTest, LeafNode) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	bptr_t root = 0;
	mem_reset_all(&ctx);
	bval_t value;

	value.data = 2;
	EXPECT_EQ(insert(&root, 0, value, &ctx), SUCCESS);
	EXPECT_EQ(mem_read(root, &ctx).keys[0], 0);
	EXPECT_EQ(mem_read(root, &ctx).values[0].data, 2);
	dump_node_list(log_stream, memory);
	ASSERT_TRUE(is_unlocked(root, log_stream, &ctx));

	value.data = 3;
	EXPECT_EQ(insert(&root, 5, value, &ctx), SUCCESS);
	EXPECT_EQ(mem_read(root, &ctx).keys[1], 5);
	EXPECT_EQ(mem_read(root, &ctx).values[1].data, 3);
	dump_node_list(log_stream, memory);
	ASSERT_TRUE(is_unlocked(root, log_stream, &ctx));

	value.data = 1;
	EXPECT_EQ(insert(&root, 3, value, &ctx), SUCCESS);
	EXPECT_EQ(mem_read(root, &ctx).keys[1], 3);
	EXPECT_EQ(mem_read(root, &ctx).values[1].data, 1);
	EXPECT_EQ(mem_read(root, &ctx).keys[2], 5);
	EXPECT_EQ(mem_read(root, &ctx).values[2].data, 3);
	dump_node_list(log_stream, memory);
	ASSERT_TRUE(is_unlocked(root, log_stream, &ctx));

	EXPECT_TRUE(validate(root, log_stream, &ctx));
	fprintf(log_stream, "\n\n");
}

TEST(InsertTest, SplitRoot) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	bptr_t root = 0;
	bptr_t lchild;
	bval_t value;
	mem_reset_all(&ctx);

	value.data = 0;
	EXPECT_EQ(insert(&root, -value.data, value, &ctx), SUCCESS);
	dump_node_list(log_stream, memory);
	EXPECT_EQ(mem_read(root, &ctx).keys[0], -value.data);
	EXPECT_EQ(mem_read(root, &ctx).values[0].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream, &ctx));
	value.data = -5;
	EXPECT_EQ(insert(&root, -value.data, value, &ctx), SUCCESS);
	dump_node_list(log_stream, memory);
	EXPECT_EQ(mem_read(root, &ctx).keys[1], -value.data);
	EXPECT_EQ(mem_read(root, &ctx).values[1].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream, &ctx));
	value.data = -3;
	EXPECT_EQ(insert(&root, -value.data, value, &ctx), SUCCESS);
	dump_node_list(log_stream, memory);
	EXPECT_EQ(mem_read(root, &ctx).keys[1], -value.data);
	EXPECT_EQ(mem_read(root, &ctx).values[1].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream, &ctx));
	// This one causes a split
	value.data = -1;
	EXPECT_EQ(insert(&root, -value.data, value, &ctx), SUCCESS);
	dump_node_list(log_stream, memory);
	lchild = mem_read(root, &ctx).values[0].ptr;
	EXPECT_EQ(mem_read(lchild, &ctx).keys[1], -value.data);
	EXPECT_EQ(mem_read(lchild, &ctx).values[1].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream, &ctx));
	value.data = -4;
	EXPECT_EQ(insert(&root, -value.data, value, &ctx), SUCCESS);
	dump_node_list(log_stream, memory);
	ASSERT_TRUE(is_unlocked(root, log_stream, &ctx));
	ASSERT_TRUE(is_unlocked(lchild, log_stream, &ctx));

	EXPECT_TRUE(validate(root, log_stream, &ctx));
	fprintf(log_stream, "\n\n");
}

TEST(InsertTest, SequentialInsert) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	bptr_t root = 0;
	bval_t value;
	mem_reset_all(&ctx);

	// Insert values
	for (uint_fast8_t i = 1; i <= (TREE_ORDER/2)*(MAX_LEAVES+1); ++i) {
		value.data = -i;
		ASSERT_EQ(insert(&root, i, value, &ctx), SUCCESS);
		dump_node_list(log_stream, memory);
		ASSERT_TRUE(is_unlocked(root, log_stream, &ctx));
	}
	EXPECT_TRUE(check_inserted_leaves());

	EXPECT_TRUE(validate(root, log_stream, &ctx));
	fprintf(log_stream, "\n\n");
}


#endif
