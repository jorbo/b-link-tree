#include "split.h"
#include "memory.h"
#include "node.h"
#include <string.h>


//! @brief Divide by 2 and take the ceiling using only integer arithmetic
//! @param[in] x  The value to operate on
//! @return ceil(x/2)
#define DIV2CEIL(x) (((x) & 1) ? (((x)/2) + 1) : ((x)/2))


//! @brief Clear a node's keys
//! @param[in] node  The node whose keys should be cleared
inline static void init_node(Node *node) {
	memset(node->keys, INVALID, TREE_ORDER * sizeof(bkey_t));
}

//! @brief Allocate a new sibling node in an empty slot in local memory
//!
//! Acquires a lock on the sibling node.  The sibling is always allocated on
//! the same machine as the leaf being split (ctx->local_id).
static ErrorCode alloc_sibling(
	//! [in] Root of the tree the nodes reside in
	bptr_t const *root,
	//! [in] The node to split
	AddrNode *leaf,
	//! [out] The contents of the split node's new sibling
	AddrNode *sibling,
	//! [in] Memory context
	mem_context_t *ctx HBM_PARAM
) {
	(void)root;
	const uint_fast8_t level = get_level(leaf->addr);
	bool success;
	bptr_t laddr;

	// Find an empty local slot at the same level as the node being split
	for (laddr = level * MAX_NODES_PER_LEVEL;
		laddr < (level+1) * MAX_NODES_PER_LEVEL;
		++laddr) {
		sibling->addr = bptr_make(ctx->local_id, laddr);
		sibling->node = mem_read_trylock(sibling->addr, ctx, &success HBM_ARG);
		if (success && sibling->node.keys[0] == INVALID) {
			break;
		} else {
			mem_unlock(sibling->addr, ctx HBM_ARG);
		}
	}
	// If we didn't break, we didn't find an empty slot
	if (laddr == (level+1) * MAX_NODES_PER_LEVEL) {
		sibling->addr = INVALID;
		return OUT_OF_MEMORY;
	}
	// Adjust next node pointers
	sibling->node.next = leaf->node.next;
	leaf->node.next = sibling->addr;
	// Move half of old node's contents to new node
	for (li_t i = 0; i < TREE_ORDER/2; ++i) {
		sibling->node.keys[i] = leaf->node.keys[i + (TREE_ORDER/2)];
		sibling->node.values[i] = leaf->node.values[i + (TREE_ORDER/2)];
		leaf->node.keys[i + (TREE_ORDER/2)] = INVALID;
	}

	return SUCCESS;
}


//! @brief Assign an allocated sibling pair at the root leve of the tree
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode split_root(
	//! [in] Root of the tree the nodes reside in
	bptr_t *root,
	//! [in] The node to split
	AddrNode const *leaf,
	//! [inout] The parent of the node to split
	AddrNode *parent,
	//! [in] The contents of the split node's new sibling
	AddrNode const *sibling,
	//! [in] Memory context
	mem_context_t *ctx HBM_PARAM
) {
	// If this is the only node we need to create the first inner node
	if (is_leaf(leaf->addr)) {
		// New root is the first inner-node slot, encoded with the local node_id
		*root = bptr_make(ctx->local_id, MAX_LEAVES);
	} else {
		bptr_t local_root = bptr_local_addr(*root);
		if (local_root + MAX_NODES_PER_LEVEL >= MEM_SIZE) {
			return OUT_OF_MEMORY;
		} else {
			*root = bptr_make(ctx->local_id, local_root + MAX_NODES_PER_LEVEL);
		}
	}
	parent->addr = *root;
	parent->node = mem_read_lock(parent->addr, ctx HBM_ARG);
	init_node(&parent->node);
	parent->node.keys[0] = leaf->node.keys[DIV2CEIL(TREE_ORDER)-1];
	parent->node.values[0].ptr = leaf->addr;
	parent->node.keys[1] = sibling->node.keys[(TREE_ORDER/2)-1];
	parent->node.values[1].ptr = sibling->addr;
	return SUCCESS;
}


//! @brief Assign an allocated sibling pair below the root leve of the tree
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode split_nonroot(
	//! [in] Root of the tree the nodes reside in
	bptr_t const *root,
	//! [in] The node to split
	AddrNode const *leaf,
	//! [inout] The parent of the node to split
	AddrNode *parent,
	//! [in] The contents of the split node's new sibling
	AddrNode const *sibling
) {
	(void)root;
	if (is_full(&parent->node)) {
		return PARENT_FULL;
	} else {
		for (li_t i = 0; i < TREE_ORDER; ++i) {
			// Update key of old node
			if (parent->node.values[i].ptr == leaf->addr) {
				parent->node.keys[i] = leaf->node.keys[DIV2CEIL(TREE_ORDER)-1];
				// Scoot over other nodes to fit in new node
				for (li_t j = TREE_ORDER-1; j > i; --j) {
					parent->node.keys[j] = parent->node.keys[j-1];
					parent->node.values[j] = parent->node.values[j-1];
				}
				// Insert new node
				parent->node.keys[i+1] = sibling->node.keys[(TREE_ORDER/2)-1];
				parent->node.values[i+1].ptr = sibling->addr;
				return SUCCESS;
			}
		}
		return NOT_IMPLEMENTED;
	}
}


ErrorCode split_node(
	bptr_t *root, AddrNode *leaf, AddrNode *parent, AddrNode *sibling, mem_context_t *ctx HBM_PARAM
) {
	ErrorCode status = alloc_sibling(root, leaf, sibling, ctx HBM_ARG);
	if (status != SUCCESS) return status;
	if (parent->addr == INVALID) {
		status = split_root(root, leaf, parent, sibling, ctx HBM_ARG);
	} else {
		status = split_nonroot(root, leaf, parent, sibling);
	}
	if (status == SUCCESS) {
		#ifdef OPTIMISTIC_LOCK
		if (!mem_write_unlock(parent, ctx HBM_ARG)) status = RESTART;
		#else
		mem_write_unlock(parent, ctx HBM_ARG);
		#endif
	}
	return status;
}
