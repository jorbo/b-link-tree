#ifndef INSERT_H
#define INSERT_H

#include "types.h"

typedef struct Node Node;

//! @brief Insert a new value into the tree with the given key and value
//! @param[inout] root   The address of the root of the tree to insert into
//! @param[in]    key    The key under which the value should be inserted
//! @param[in]    value  The value to insert
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode insert(bptr_t *root, bkey_t key, bval_t value, Node **memory);

#endif
