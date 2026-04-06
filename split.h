#ifndef SPLIT_H
#define SPLIT_H


#include "types.h"

typedef struct Node Node;
typedef struct AddrNode AddrNode;


//! @brief Split a node in the tree and return the affected nodes
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode split_node(
	//! [inout] Root of the tree the nodes reside in
	bptr_t *root,
	//! [in] The node to split
	AddrNode *leaf,
	//! [inout] The parent of the node to split
	AddrNode *parent,
	//! [out] The contents of the split node's new sibling
	AddrNode *sibling,
	Node **memory
);


#endif
