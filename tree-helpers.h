#ifndef TREE_HELPERS_H
#define TREE_HELPERS_H


#include "types.h"

typedef struct Node Node;


//! @brief Get the index of a leaf in a lineage array
//! @param[in] lineage  An existing array of a node's parents up until the root
//! @return Index of the leaf within the lineage array
inline static uint_fast8_t get_leaf_idx(bptr_t const *lineage) {
	for (uint_fast8_t i = MAX_LEVELS-1; i > 0; i--) {
		if (lineage[i] != INVALID) return i;
	}
	return 0;
}


//! @brief Helper function for traversal of a tree, used for search and insert
//! @param[in]  tree     The tree to search
//! @param[in]  key      The key to search for
//! @param[out] lineage  Pointer array to hold the parents of all nodes on the
//!                      path to the given leaf. Should be preallocated and
//!                      large enough to accommodate a tree of maximum height.
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode trace_lineage(bptr_t root, bkey_t key, bptr_t *lineage, Node const **memory);


#endif
