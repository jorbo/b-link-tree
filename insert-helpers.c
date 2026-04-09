#include "insert-helpers.h"
#include "memory.h"
#include "node.h"


bkey_t max(Node const *node) {
	for (li_t i = TREE_ORDER-1; i > 0; --i) {
		if (node->keys[i] != INVALID) return node->keys[i];
	}
	return node->keys[0];
}


ErrorCode insert_nonfull(Node *node, bkey_t key, bval_t value) {
	li_t i_insert = 0;

	for (li_t i = 0; i < TREE_ORDER; ++i) {
		// Found an empty slot
		// Will be the last slot
		if (node->keys[i] == INVALID) {
			// Scoot nodes if necessary to maintain ordering
			// Iterate right to left from the last node to the insertion point
			for (; i_insert < i; i--) {
				node->keys[i] = node->keys[i-1];
				node->values[i] = node->values[i-1];
			}
			// Do the actual insertion
			node->keys[i_insert] = key;
			node->values[i_insert] = value;
			return SUCCESS;
		} else if (node->keys[i] == key) {
			return KEY_EXISTS;
		} else if (node->keys[i] < key) {
			i_insert++;
		}
	}
	return OUT_OF_MEMORY;
}


ErrorCode insert_after_split(
	bkey_t key, bval_t value, AddrNode *leaf, AddrNode *sibling, mem_context_t *ctx
) {
	ErrorCode status;
	if (key < max(&leaf->node)) {
		status = insert_nonfull(&leaf->node, key, value);
	} else {
		status = insert_nonfull(&sibling->node, key, value);
	}
#ifdef OPTIMISTIC_LOCK
	if (status != SUCCESS) return status;
	if (
		!mem_write_unlock(sibling, ctx) || !mem_write_unlock(leaf, ctx)
	) return RESTART;
	return SUCCESS;
#else
	mem_write_unlock(sibling, ctx);
	mem_write_unlock(leaf, ctx);
	return status;
#endif
}


ErrorCode rekey(Node *node, bkey_t old_key, bkey_t new_key) {
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (node->keys[i] == old_key) {
			node->keys[i] = new_key;
			return SUCCESS;
		}
	}
	return NOT_FOUND;
}
