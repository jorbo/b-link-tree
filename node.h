#ifndef NODE_H
#define NODE_H


#include "lock.h"
#include "types.h"


//! @brief A generic node within the tree
//!
//! Can be a leaf node or an inner node
struct Node {
	//! @brief Keys corresponding to child data at the same indices
	//! @par Inner Nodes
	//! In inner nodes, the keys are the exclusive upper
	//! bounds to ranges of child keys such that
	//! `keys[i-1] <= child[i].keys < keys[i]`
	//! For the 0th key, \f$-\infty\f$ is the bound.
	//! @par Leaf Nodes
	//! In leaf nodes the keys are exact lookup values.
	bkey_t keys[TREE_ORDER];
	//! @brief "Pointer to" (address of) the next largest sibling node
	//!
	//! The @ref bval_t union is used to select how they are interpreted
	//! (as internal nodes that point to other nodes or leaves that hold
	//! values)
	bval_t values[TREE_ORDER];
	//! @brief The values corresponding to the keys at the same indices
	//!
	//! These may be leaf data or pointers within the tree.
	bptr_t next;
	//! @brief Used to restrict concurrent modifications to this node
	lock_t lock;
};
typedef struct Node Node;

//! @brief Traverse the tree structure in search of the given key
//! @param[in] key The key to search for
//! @return A result containing a status code for success/failure of the
//!         operation along with the address of the next node to check on
//!         success. If this is a leaf node, its data will be returned.
bstatusval_t find_next(Node const *n, bkey_t key);
//! @brief Find the value corresponding to a given key
bstatusval_t find_value(Node const *n, bkey_t key);
bool has_value(Node const *n, bval_t value);
//! @brief "Is empty", returns true for unallocated memory
bool is_valid(Node const *n);
//! @brief Check if all keys in a node are in use
//! @param[in] node  The node to check
//! @return True if all keys are in use, false otherwise
bool is_full(Node const *n);
//! @brief Empty this node's contents and restore its default state
void clear(Node *n);


//! @brief A node that knows the address where it resides in the tree
struct AddrNode {
	Node node;
	bptr_t addr;
};
typedef struct AddrNode AddrNode;

//! @brief Check if a node at the given address is a leaf node or an inner node
//! @param[in] addr  Address of the node within the tree to check
inline static bool is_leaf(bptr_t addr) {
	// Leaves are stored at lowest local addresses; strip node_id before comparing
	return bptr_local_addr(addr) < MAX_LEAVES;
}

//! @brief Check which level of the tree a node address resides on
//! Assumes all levels take up equal space in memory
//! @param[in] node_ptr  The node address to check
inline static bptr_t get_level(bptr_t node_ptr) {
	return (bptr_local_addr(node_ptr) / MAX_NODES_PER_LEVEL);
}


#endif
