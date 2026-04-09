#ifndef INSERT_HELPERS_H
#define INSERT_HELPERS_H


#include "types.h"
typedef struct Node Node;
typedef struct AddrNode AddrNode;
typedef struct mem_context mem_context_t;


//! @brief Find the maximum key in a node
//! @param[in] node  The node to check
//! @return The largest valid key in the node
bkey_t max(Node const *node);

//! @brief Insert into a non-full leaf node
//! @param[in] node   The node to insert into
//! @param[in] key    The key to insert
//! @param[in] value  The value to insert
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode insert_nonfull(Node *node, bkey_t key, bval_t value);

//! @brief Insert new data into a node or its newly created sibling
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode insert_after_split(
	//! [in] The key to insert
	bkey_t key,
	//! [in] The value to insert
	bval_t value,
	//! [out] The original node to insert into
	AddrNode *leaf,
	//! [out] The new sibling to insert into
	AddrNode *sibling,
	//! [in] Memory context
	mem_context_t *ctx
);

//! @brief Replace a key without changing its corresponding value
//!
//! Helper for adjusting the high key after splitting nodes
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode rekey(Node *node, bkey_t old_key, bkey_t new_key);


#endif
