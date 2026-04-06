#ifndef TESTS_SEARCH_HPP
#define TESTS_SEARCH_HPP


extern "C" {
#include "../io.h"
#include "../memory.h"
#include "../node.h"
#include "../search.h"
#include "../validate.h"
};
#include <gtest/gtest.h>

extern FILE *log_stream;
extern Node *memory[MAX_LEVELS];


#if TREE_ORDER >= 4
TEST(SearchTest, RootIsLeaf) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	AddrNode root;
	root.addr = bptr_make(0, 0);
	mem_reset_all(memory);
	root.node = mem_read_lock(root.addr, memory);
	bstatusval_t result;

	root.node.keys[0] = 1; root.node.values[0].data = -1;
	root.node.keys[1] = 2; root.node.values[1].data = -2;
	root.node.keys[2] = 4; root.node.values[2].data = -4;
	root.node.keys[3] = 5; root.node.values[3].data = -5;
	mem_write_unlock(&root, memory);
	dump_node_list(log_stream, memory);
	EXPECT_EQ(search(root.addr, 0, memory).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 3, memory).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 6, memory).status, NOT_FOUND);
	result = search(root.addr, 1, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -1);
	result = search(root.addr, 2, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -2);
	result = search(root.addr, 4, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -4);
	result = search(root.addr, 5, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -5);

	EXPECT_TRUE(validate(root.addr, log_stream, memory));
	EXPECT_TRUE(is_unlocked(root.addr, log_stream, memory));
	fprintf(log_stream, "\n\n");
}
#endif

#if TREE_ORDER == 4
TEST(SearchTest, OneInternal) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	AddrNode root;
	root.addr = bptr_make(1, 0);
	mem_reset_all(memory);
	root.node = mem_read_lock(root.addr, memory);
	AddrNode lchild = {.node = mem_read_lock(bptr_make(0, 0), memory), .addr = bptr_make(0, 0)};
	AddrNode rchild = {.node = mem_read_lock(bptr_make(0, 1), memory), .addr = bptr_make(0, 1)};
	bstatusval_t result;

	root.node.keys[0] = 6; root.node.values[0].ptr = lchild.addr;
	root.node.keys[1] = 12; root.node.values[1].ptr = rchild.addr;
	lchild.node.keys[0] = 1; lchild.node.values[0].data = -1;
	lchild.node.keys[1] = 2; lchild.node.values[1].data = -2;
	lchild.node.keys[2] = 4; lchild.node.values[2].data = -4;
	lchild.node.keys[3] = 5; lchild.node.values[3].data = -5;
	lchild.node.next = rchild.addr;
	rchild.node.keys[0] = 7; rchild.node.values[0].data = -7;
	rchild.node.keys[1] = 8; rchild.node.values[1].data = -8;
	rchild.node.keys[2] = 10; rchild.node.values[2].data = -10;
	rchild.node.keys[3] = 11; rchild.node.values[3].data = -11;
	mem_write_unlock(&root, memory);
	mem_write_unlock(&lchild, memory);
	mem_write_unlock(&rchild, memory);
	dump_node_list(log_stream, memory);
	EXPECT_EQ(search(root.addr, 0, memory).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 3, memory).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 6, memory).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 9, memory).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 12, memory).status, NOT_FOUND);
	result = search(root.addr, 1, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -1);
	result = search(root.addr, 2, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -2);
	result = search(root.addr, 4, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -4);
	result = search(root.addr, 5, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -5);
	result = search(root.addr, 7, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -7);
	result = search(root.addr, 8, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -8);
	result = search(root.addr, 10, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -10);
	result = search(root.addr, 11, memory);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -11);

	EXPECT_TRUE(validate(root.addr, log_stream, memory));
	EXPECT_TRUE(is_unlocked(root.addr, log_stream, memory));
	fprintf(log_stream, "\n\n");
}
#endif


#endif
