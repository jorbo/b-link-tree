#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

typedef struct Node Node;

//! @brief Search a tree for a key
//! @param[in]  root   The root of the tree to search
//! @param[in]  key    The key to search for
//! @return Struct containing requested data on success and an error code
bstatusval_t search(bptr_t root, bkey_t key, Node const **memory);

#endif
