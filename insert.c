#include "insert.h"
#include "insert-helpers.h"
#include "memory.h"
#include "node.h"
#include "split.h"
#include "tree-helpers.h"
#include <string.h>


#ifdef OPTIMISTIC_LOCK
static ErrorCode optimistic_insert(bptr_t *root, bkey_t key, bval_t value, mem_context_t *ctx HBM_PARAM) {
#else
ErrorCode insert(bptr_t *root, bkey_t key, bval_t value, mem_context_t *ctx HBM_PARAM) {
#endif
	ErrorCode status;
	li_t i_leaf;
	AddrNode leaf, parent, sibling;
	bptr_t lineage[MAX_LEVELS];
	bool keep_splitting = false;

	// Initialize lineage array
	memset(lineage, INVALID, MAX_LEVELS*sizeof(bptr_t));
	// Try to trace lineage
	status = trace_lineage(*root, key, lineage, ctx HBM_ARG);
	if (status != SUCCESS) return status;
	// Load leaf
	i_leaf = get_leaf_idx(lineage);
	leaf.addr = lineage[i_leaf];
	leaf.node = mem_read_lock(leaf.addr, ctx HBM_ARG);
	do {
		// Load this node's parent, if it exists
		if (i_leaf > 0) {
			parent.addr = lineage[i_leaf-1];
			parent.node = mem_read(parent.addr, ctx HBM_ARG);
			// If parent has shifted since our initial traversal
			while (!has_value(&parent.node, (bval_t) leaf.addr)) {
				mem_unlock(parent.addr, ctx HBM_ARG);
				parent.addr = parent.node.next;
				parent.node = mem_read(parent.addr, ctx HBM_ARG);
				if (parent.node.next == INVALID) return NOT_FOUND;
			}
			parent.node = mem_read_lock(parent.addr, ctx HBM_ARG);
		} else {
			parent.addr = INVALID;
		}

		if (!is_full(&leaf.node)) {
			status = insert_nonfull(&leaf.node, key, value);
			#ifdef OPTIMISTIC_LOCK
			if (!mem_write_unlock(&leaf, ctx HBM_ARG)) return RESTART;
			#else
			mem_write_unlock(&leaf, ctx HBM_ARG);
			#endif
			if (parent.addr != INVALID) mem_unlock(parent.addr, ctx HBM_ARG);
			if (status != SUCCESS) return status;
		} else {
			// Try to split this node
			status = split_node(root, &leaf, &parent, &sibling, ctx HBM_ARG);
			keep_splitting = (status == PARENT_FULL);
			// Unrecoverable failure
			if (status != SUCCESS && status != PARENT_FULL) {
				mem_unlock(leaf.addr, ctx HBM_ARG);
				if (sibling.addr != INVALID) mem_unlock(sibling.addr, ctx HBM_ARG);
				mem_unlock(parent.addr, ctx HBM_ARG);
				return status;
			}
			// Insert the new content and unlock leaf and its sibling
			status = insert_after_split(key, value, &leaf, &sibling, ctx HBM_ARG);
			if (keep_splitting) {
				// Try this again on the parent
				key = max(&sibling.node);
				rekey(&parent.node, key, max(&leaf.node));
				value.ptr = sibling.addr;
				i_leaf--;
				leaf = parent;
			} else if (status != SUCCESS) {
				mem_unlock(parent.addr, ctx HBM_ARG);
				return status;
			}
		}
	} while (keep_splitting);

	return SUCCESS;
}

#ifdef OPTIMISTIC_LOCK
ErrorCode insert(bptr_t *root, bkey_t key, bval_t value, mem_context_t *ctx HBM_PARAM) {
	ErrorCode status;
	do {
		status = optimistic_insert(root, key, value, ctx HBM_ARG);
	} while (status == RESTART);
	return status;
}
#endif
