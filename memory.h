#ifndef MEMORY_H
#define MEMORY_H


#include "types.h"
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

typedef struct Node Node;
typedef struct AddrNode AddrNode;


#ifndef __SYNTHESIS__
//! @brief A remote node entry: maps a node_id to a Node memory slab.
//!
//! In simulation this is a direct pointer to another in-process array.
//! In a real RDMA deployment, replace @ref memory with a connection handle.
typedef struct {
	node_id_t  id;
	Node      *memory;
} remote_node_t;

//! Maximum number of distinct network nodes the tree can span
#define MAX_REMOTE_NODES 256u
#endif /* !__SYNTHESIS__ */

//! @brief Memory context: holds the local memory slab and a table of remote
//!        nodes reachable via RDMA.
//!
//! Pass a pointer to this context wherever tree operations need memory access.
//! Use @ref mem_context_local to create a single-node context for testing.
struct mem_context {
	//! This machine's node_id
	node_id_t     local_id;
#ifndef __SYNTHESIS__
	//! Pointer to this machine's local Node array (size MEM_SIZE).
	//! Not present in synthesis — HBM is threaded as a separate parameter.
	Node         *local_memory;
	//! Table of remote nodes indexed by node_id (simulation only).
	remote_node_t remotes[MAX_REMOTE_NODES];
#endif
};
typedef struct mem_context mem_context_t;

//! @brief Initialise a context for single-node operation.
//! @param[in] id      The local node_id (use 0 for single-node / test use)
//! @param[in] memory  Pointer to the local Node array
#ifdef __SYNTHESIS__
//! In synthesis @p memory is NOT stored in the context; it is forwarded via
//! the HBM_PARAM / HBM_ARG macros throughout the memory API.
static inline mem_context_t mem_context_local(node_id_t id, Node *memory) {
	(void)memory;
	mem_context_t ctx;
	ctx.local_id = id;
	return ctx;
}
//! Extra HBM array parameter appended to every memory-API function in synthesis.
#define HBM_PARAM , Node *hbm
//! Matching argument to forward the HBM pointer at each call site.
#define HBM_ARG   , hbm
#else
mem_context_t mem_context_local(node_id_t id, Node *memory);
//! In simulation these macros expand to nothing.
#define HBM_PARAM
#define HBM_ARG
#endif /* __SYNTHESIS__ */


//! @brief Read a node from memory without grabbing any locks
Node mem_read(bptr_t address, mem_context_t *ctx HBM_PARAM);

//! @brief Read a node from memory, locking it in memory
Node mem_read_lock(bptr_t address, mem_context_t *ctx HBM_PARAM);

//! @brief Read a node from memory, locking it in memory if possible,
//         otherwise setting success to false
Node mem_read_trylock(bptr_t address, mem_context_t *ctx, bool *success HBM_PARAM);


#ifdef OPTIMISTIC_LOCK
//! @brief Write a node to memory and unlock it
//! @return true on success, false on failure
bool mem_write_unlock(AddrNode *node, mem_context_t *ctx HBM_PARAM);
#else
//! @brief Write a node to memory and unlock it
void mem_write_unlock(AddrNode *node, mem_context_t *ctx HBM_PARAM);
#endif

//! @brief Unlock a node in memory
//!
//! Used when a node has been read and locked, but should be unlocked without
//! modification. For example, if an error has been detected before a
//! modification operation completes.
void mem_unlock(bptr_t address, mem_context_t *ctx HBM_PARAM);

//! @brief Reset the local memory slab to a slate of blank nodes
//!
//! All data is 1s except for locks
void mem_reset_all(mem_context_t *ctx HBM_PARAM);


#endif
