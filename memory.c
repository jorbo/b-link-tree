#include "memory.h"
#include "lock.h"
#include "node.h"
#include <assert.h>
#include <string.h>


static lock_t local_readlock = LOCK_INIT;


//! @brief Select the Node array for the given encoded address.
//!        For local addresses uses ctx->local_memory;
//!        for remote addresses uses ctx->remotes[node_id].memory.
static Node *resolve_mem(bptr_t address, mem_context_t *ctx) {
	node_id_t nid = bptr_node_id(address);
	if (nid == ctx->local_id) {
		return ctx->local_memory;
	} else {
		assert(ctx->remotes[nid].memory != NULL);
		return ctx->remotes[nid].memory;
	}
}

mem_context_t mem_context_local(node_id_t id, Node *memory) {
	mem_context_t ctx;
	memset(&ctx, 0, sizeof(ctx));
	ctx.local_id = id;
	ctx.local_memory = memory;
	return ctx;
}

Node mem_read(bptr_t address, mem_context_t *ctx) {
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx);
	assert(laddr < MEM_SIZE);
	return mem[laddr];
}

//! The HLS interface to HBM does not support atomic test-and-set operations.
//! Atomic test-and-set operations within the FPGA fabric are allowed.
//! Serializing test-and-set operations in memory with mutual exclusion ensures
//! that race conditions do not lead to unexpected concurrent modification.
//!
//! @todo Set up multiple locks for specific regions of memory, such as by
//! address ranges or hashes to allow higher write bandwidth.
Node mem_read_lock(bptr_t address, mem_context_t *ctx) {
#ifdef OPTIMISTIC_LOCK
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx);
	return mem[laddr];
#else
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx);
	Node tmp;

	assert(laddr < MEM_SIZE);
	// Read the given address from main memory until its lock is released
	// Then grab the lock
	do {
		lock_p(&local_readlock);
		tmp = mem[laddr];
		if (test_and_set(&tmp.lock) == 0) {
			break;
		} else {
			lock_v(&local_readlock);
		}
	} while(true);
	// Write back the locked value to main memory
	mem[laddr] = tmp;
	// Release the local lock for future writers
	lock_v(&local_readlock);
	return tmp;
#endif
}

Node mem_read_trylock(bptr_t address, mem_context_t *ctx, bool *success) {
#ifdef OPTIMISTIC_LOCK
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx);
	return mem[laddr];
#else
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx);
	Node tmp;

	assert(laddr < MEM_SIZE);
	assert(success != NULL);
	// Read the given address from main memory until its lock is released
	// Then grab the lock
	lock_p(&local_readlock);
	tmp = mem[laddr];
	*success = (test_and_set(&tmp.lock) == 0);
	if (*success) {
		// Write back the locked value to main memory
		mem[laddr] = tmp;
	}
	// Release the local lock for future writers
	lock_v(&local_readlock);
	return tmp;
#endif
}

#ifdef OPTIMISTIC_LOCK
bool mem_write_unlock(AddrNode *node, mem_context_t *ctx) {
	bptr_t laddr = bptr_local_addr(node->addr);
	Node *mem = resolve_mem(node->addr, ctx);
	assert(laddr < MEM_SIZE);
	Node tmp = mem[laddr];
	if (tmp.lock != node->node.lock) return false;
	node->node.lock++;
	mem[laddr] = node->node;
	return true;
}
#else
void mem_write_unlock(AddrNode *node, mem_context_t *ctx) {
	bptr_t laddr = bptr_local_addr(node->addr);
	Node *mem = resolve_mem(node->addr, ctx);
	assert(laddr < MEM_SIZE);
	lock_v(&node->node.lock);
	mem[laddr] = node->node;
}
#endif

void mem_unlock(bptr_t address, mem_context_t *ctx) {
#ifndef OPTIMISTIC_LOCK
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx);
	assert(laddr < MEM_SIZE);
	// Cast byte pointer to lock_t pointer to ensure write is of correct size
	*((lock_t*) (
		// Byte pointer
		&(
			(uint8_t*) mem
		)[
			// Address of the lock field of the node who starts at laddr
			(laddr+1)*sizeof(Node)-sizeof(lock_t)
		]
	)) = LOCK_INIT;
#endif
}

void mem_reset_all(mem_context_t *ctx) {
	memset(ctx->local_memory, INVALID, MEM_SIZE*sizeof(Node));
	for (bptr_t i = 0; i < MEM_SIZE; i++) {
		ctx->local_memory[i].lock = LOCK_INIT;
	}
}
