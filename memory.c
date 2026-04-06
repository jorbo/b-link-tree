#include "memory.h"
#include "lock.h"
#include "node.h"
#include <assert.h>
#include <string.h>


static lock_t local_readlock = LOCK_INIT;


Node mem_read(bptr_t address, Node const **memory) {
	assert(bptr_node(address) < MAX_LEVELS);
	assert(bptr_addr(address) < MAX_NODES_PER_LEVEL);
	return memory[bptr_node(address)][bptr_addr(address)];
}

//! The HLS interface to HBM does not support atomic test-and-set operations.
//! Atomic test-and-set operations within the FPGA fabric are allowed.
//! Serializing test-and-set operations in memory with mutual exclusion ensures
//! that race conditions do not lead to unexpected concurrent modification.
//!
//! @todo Set up multiple locks for specific regions of memory, such as by
//! address ranges or hashes to allow higher write bandwidth.
Node mem_read_lock(bptr_t address, Node **memory) {
#ifdef OPTIMISTIC_LOCK
	return memory[bptr_node(address)][bptr_addr(address)];
#else
	Node tmp;

	assert(bptr_node(address) < MAX_LEVELS);
	assert(bptr_addr(address) < MAX_NODES_PER_LEVEL);

	// Read the given address from main memory until its lock is released
	// Then grab the lock
	do {
		lock_p(&local_readlock);
		tmp = memory[bptr_node(address)][bptr_addr(address)];
		if (test_and_set(&tmp.lock) == 0) {
			break;
		} else {
			lock_v(&local_readlock);
		}
	} while(true);
	// Write back the locked value to main memory
	memory[bptr_node(address)][bptr_addr(address)] = tmp;
	// Release the local lock for future writers
	lock_v(&local_readlock);
	return tmp;
#endif
}

Node mem_read_trylock(bptr_t address, Node **memory, bool *success) {
#ifdef OPTIMISTIC_LOCK
	return memory[bptr_node(address)][bptr_addr(address)];
#else
	Node tmp;

	assert(bptr_node(address) < MAX_LEVELS);
	assert(bptr_addr(address) < MAX_NODES_PER_LEVEL);
	assert(success != NULL);
	// Read the given address from main memory until its lock is released
	// Then grab the lock
	lock_p(&local_readlock);
	tmp = memory[bptr_node(address)][bptr_addr(address)];
	*success = (test_and_set(&tmp.lock) == 0);
	if (*success) {
		// Write back the locked value to main memory
		memory[bptr_node(address)][bptr_addr(address)] = tmp;
	}
	// Release the local lock for future writers
	lock_v(&local_readlock);
	return tmp;
#endif
}

#ifdef OPTIMISTIC_LOCK
bool mem_write_unlock(AddrNode *node, Node **memory) {
	assert(bptr_node(address) < MAX_LEVELS);
	assert(bptr_addr(address) < MAX_NODES_PER_LEVEL);
	Node tmp = memory[bptr_node(node->addr)][bptr_addr(node->addr)];
	if (tmp.lock != node->node.lock) return false;
	node->node.lock++;
	memory[bptr_node(node->addr)][bptr_addr(node->addr)] = node->node;
	return true;
}
#else
void mem_write_unlock(AddrNode *node, Node **memory) {
	assert(bptr_node(node->addr) < MAX_LEVELS);
	assert(bptr_addr(node->addr) < MAX_NODES_PER_LEVEL);
	lock_v(&node->node.lock);
	memory[bptr_node(node->addr)][bptr_addr(node->addr)] = node->node;
}
#endif

void mem_unlock(bptr_t address, Node **memory) {
#ifndef OPTIMISTIC_LOCK
	assert(bptr_node(address) < MAX_LEVELS);
	assert(bptr_addr(address) < MAX_NODES_PER_LEVEL);
	memory[bptr_node(address)][bptr_addr(address)].lock = LOCK_INIT;
#endif
}

void mem_reset_all(Node **memory) {
	for (bptr_t i = 0; i < MAX_LEVELS; i++) {
		memset(memory[i], INVALID, MAX_NODES_PER_LEVEL * sizeof(Node));
		for (bptr_t j = 0; j < MAX_NODES_PER_LEVEL; j++) {
			memory[i][j].lock = LOCK_INIT;
		}
	}
}
