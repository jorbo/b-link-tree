#include "tree-helpers.h"
#include "memory.h"
#include "node.h"


//! @brief Helper function for traversal of a tree, used for search and insert
//! @param[in]  tree     The tree to search
//! @param[in]  key      The key to search for
//! @param[out] lineage  Pointer array to hold the parents of all nodes on the
//!                      path to the given leaf. Should be preallocated and
//!                      large enough to accommodate a tree of maximum height.
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode trace_lineage(bptr_t root, bkey_t key, bptr_t *lineage, mem_context_t *ctx HBM_PARAM) {
	lineage[0] = root;
	li_t curr = 0;
	Node node;
	bstatusval_t result;

	// Iterate until we hit a leaf
	while (!is_leaf(lineage[curr])) {
		node = mem_read(lineage[curr], ctx HBM_ARG);
		result = find_next(&node, key);
		if (result.status != SUCCESS) return result.status;
		lineage[++curr] = result.value.ptr;
	}

	return SUCCESS;
}
