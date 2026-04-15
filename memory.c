#include "memory.h"
#include "lock.h"
#include "node.h"
#ifndef __SYNTHESIS__
#include <assert.h>
#include <string.h>
#else
#define assert(x) ((void)0)
#endif


static lock_t local_readlock = LOCK_INIT;


#ifdef __SYNTHESIS__
//! In synthesis only local memory exists; remote nodes are reached via RDMA streams.
static Node *resolve_mem(bptr_t address, mem_context_t *ctx, Node *hbm) {
	(void)address; (void)ctx;
	return hbm;
}
#else
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
#endif /* __SYNTHESIS__ */

#ifndef __SYNTHESIS__
mem_context_t mem_context_local(node_id_t id, Node *memory) {
	mem_context_t ctx;
	memset(&ctx, 0, sizeof(ctx));
	ctx.local_id = id;
	ctx.local_memory = memory;
	return ctx;
}
#endif /* !__SYNTHESIS__ */

Node mem_read(bptr_t address, mem_context_t *ctx HBM_PARAM) {
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx HBM_ARG);
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
Node mem_read_lock(bptr_t address, mem_context_t *ctx HBM_PARAM) {
#if defined(OPTIMISTIC_LOCK) || defined(__SYNTHESIS__)
	/* In synthesis the kernel is single-threaded; skip the spinlock loop. */
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx HBM_ARG);
	return mem[laddr];
#else
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx HBM_ARG);
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

Node mem_read_trylock(bptr_t address, mem_context_t *ctx, bool *success HBM_PARAM) {
#if defined(OPTIMISTIC_LOCK) || defined(__SYNTHESIS__)
	/* In synthesis the kernel is single-threaded; skip the spinlock loop. */
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx HBM_ARG);
	if (success) *success = true;
	return mem[laddr];
#else
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx HBM_ARG);
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
bool mem_write_unlock(AddrNode *node, mem_context_t *ctx HBM_PARAM) {
	bptr_t laddr = bptr_local_addr(node->addr);
	Node *mem = resolve_mem(node->addr, ctx HBM_ARG);
	assert(laddr < MEM_SIZE);
	Node tmp = mem[laddr];
	if (tmp.lock != node->node.lock) return false;
	node->node.lock++;
	mem[laddr] = node->node;
	return true;
}
#else
void mem_write_unlock(AddrNode *node, mem_context_t *ctx HBM_PARAM) {
	bptr_t laddr = bptr_local_addr(node->addr);
	Node *mem = resolve_mem(node->addr, ctx HBM_ARG);
	assert(laddr < MEM_SIZE);
	lock_v(&node->node.lock);
	mem[laddr] = node->node;
}
#endif

void mem_unlock(bptr_t address, mem_context_t *ctx HBM_PARAM) {
#ifndef OPTIMISTIC_LOCK
	bptr_t laddr = bptr_local_addr(address);
	Node *mem = resolve_mem(address, ctx HBM_ARG);
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

void mem_reset_all(mem_context_t *ctx HBM_PARAM) {
#ifdef __SYNTHESIS__
	(void)ctx;
	for (bptr_t i = 0; i < MEM_SIZE; i++) {
		#pragma HLS loop_tripcount max=MEM_SIZE
		for (int j = 0; j < (int)sizeof(Node); j++) {
			#pragma HLS loop_tripcount max=64
			((uint8_t*)hbm)[i*sizeof(Node)+j] = (uint8_t)INVALID;
		}
		hbm[i].lock = LOCK_INIT;
	}
#else
	memset(ctx->local_memory, INVALID, MEM_SIZE*sizeof(Node));
	for (bptr_t i = 0; i < MEM_SIZE; i++) {
		ctx->local_memory[i].lock = LOCK_INIT;
	}
#endif
}
