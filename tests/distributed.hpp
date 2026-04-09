#ifndef TESTS_DISTRIBUTED_HPP
#define TESTS_DISTRIBUTED_HPP


extern "C" {
#include "../defs.h"
#include "../insert.h"
#include "../memory.h"
#include "../node.h"
#include "../search.h"
#include "../validate.h"
};
#include <gtest/gtest.h>


// ---------------------------------------------------------------------------
// bptr_t encoding unit tests
// ---------------------------------------------------------------------------

TEST(BptrEncoding, NodeIdRoundtrip) {
	EXPECT_EQ(bptr_node_id(bptr_make(3, 15)), 3);
	EXPECT_EQ(bptr_node_id(bptr_make(0, 0)), 0);
	EXPECT_EQ(bptr_node_id(bptr_make(255, 0)), 255);
}

TEST(BptrEncoding, LocalAddrRoundtrip) {
	EXPECT_EQ(bptr_local_addr(bptr_make(3, 15)), 15u);
	EXPECT_EQ(bptr_local_addr(bptr_make(0, 0)), 0u);
	EXPECT_EQ(bptr_local_addr(bptr_make(7, MAX_LEAVES - 1)), (bptr_t)(MAX_LEAVES - 1));
}

TEST(BptrEncoding, LegacyAddressIsNodeZero) {
	// All original bptr_t values (0..MEM_SIZE-1) map to node_id=0 unchanged
	for (bptr_t i = 0; i < MEM_SIZE; ++i) {
		EXPECT_EQ(bptr_node_id(i), 0);
		EXPECT_EQ(bptr_local_addr(i), i);
	}
}

TEST(BptrEncoding, IsLeafStripsNodeId) {
	// A local_addr < MAX_LEAVES is a leaf regardless of node_id
	EXPECT_TRUE(is_leaf(bptr_make(0, 0)));
	EXPECT_TRUE(is_leaf(bptr_make(7, MAX_LEAVES - 1)));
	EXPECT_FALSE(is_leaf(bptr_make(0, MAX_LEAVES)));
	EXPECT_FALSE(is_leaf(bptr_make(7, MAX_LEAVES)));
}

TEST(BptrEncoding, GetLevelStripsNodeId) {
	// get_level should be independent of node_id
	for (node_id_t nid = 0; nid < 4; ++nid) {
		for (bptr_t laddr = 0; laddr < MEM_SIZE; ++laddr) {
			EXPECT_EQ(
				get_level(bptr_make(nid, laddr)),
				get_level(laddr)   // node_id=0, same laddr
			);
		}
	}
}


// ---------------------------------------------------------------------------
// Multi-node simulation test
//
// Two virtual nodes share the tree:
//   node 0 (ctx0) owns inner nodes (addresses MAX_LEAVES .. MEM_SIZE-1)
//   node 1 (ctx1) owns leaf nodes  (addresses 0 .. MAX_LEAVES-1)
//
// We build the tree manually so leaf pointers in inner nodes encode node_id=1.
// Then we search via ctx0 and verify cross-node traversal succeeds.
// ---------------------------------------------------------------------------

#if TREE_ORDER == 4
TEST(DistributedTest, CrossNodeSearch) {
	// Two independent memory slabs
	Node mem0[MEM_SIZE];  // node 0: will hold the root inner node
	Node mem1[MEM_SIZE];  // node 1: will hold the leaf nodes

	// Build contexts and wire them together for simulation
	mem_context_t ctx0 = mem_context_local(0, mem0);
	ctx0.remotes[1].id     = 1;
	ctx0.remotes[1].memory = mem1;

	mem_context_t ctx1 = mem_context_local(1, mem1);
	ctx1.remotes[0].id     = 0;
	ctx1.remotes[0].memory = mem0;

	mem_reset_all(&ctx0);
	mem_reset_all(&ctx1);

	// Leaf node on node 1, local address 0
	bptr_t leaf0_addr = bptr_make(1, 0);
	// Leaf node on node 1, local address 1
	bptr_t leaf1_addr = bptr_make(1, 1);
	// Root inner node on node 0, at first inner-node slot
	bptr_t root_addr = bptr_make(0, MAX_LEAVES);

	// Write leaf 0 (keys 1,2,3,4) via ctx1
	AddrNode leaf0;
	leaf0.addr = leaf0_addr;
	leaf0.node = mem_read_lock(leaf0_addr, &ctx1);
	leaf0.node.keys[0] = 1; leaf0.node.values[0].data = -1;
	leaf0.node.keys[1] = 2; leaf0.node.values[1].data = -2;
	leaf0.node.keys[2] = 3; leaf0.node.values[2].data = -3;
	leaf0.node.keys[3] = 4; leaf0.node.values[3].data = -4;
	leaf0.node.next    = leaf1_addr;
	mem_write_unlock(&leaf0, &ctx1);

	// Write leaf 1 (keys 5,6,7,8) via ctx1
	AddrNode leaf1;
	leaf1.addr = leaf1_addr;
	leaf1.node = mem_read_lock(leaf1_addr, &ctx1);
	leaf1.node.keys[0] = 5; leaf1.node.values[0].data = -5;
	leaf1.node.keys[1] = 6; leaf1.node.values[1].data = -6;
	leaf1.node.keys[2] = 7; leaf1.node.values[2].data = -7;
	leaf1.node.keys[3] = 8; leaf1.node.values[3].data = -8;
	leaf1.node.next    = (bptr_t)INVALID;
	mem_write_unlock(&leaf1, &ctx1);

	// Write root inner node on node 0 via ctx0
	AddrNode root;
	root.addr = root_addr;
	root.node = mem_read_lock(root_addr, &ctx0);
	root.node.keys[0]   = 4; root.node.values[0].ptr = leaf0_addr;
	root.node.keys[1]   = 8; root.node.values[1].ptr = leaf1_addr;
	mem_write_unlock(&root, &ctx0);

	// Search via ctx0 — inner node local, leaves remote on node 1
	bstatusval_t r;
	for (bkey_t k = 1; k <= 8; ++k) {
		r = search(root_addr, k, &ctx0);
		EXPECT_EQ(r.status, SUCCESS) << "key=" << k;
		EXPECT_EQ(r.value.data, -(int)k) << "key=" << k;
	}

	// Keys not in the tree should return NOT_FOUND
	EXPECT_EQ(search(root_addr, 0,  &ctx0).status, NOT_FOUND);
	EXPECT_EQ(search(root_addr, 9,  &ctx0).status, NOT_FOUND);
}
#endif  // TREE_ORDER == 4


#endif  // TESTS_DISTRIBUTED_HPP
