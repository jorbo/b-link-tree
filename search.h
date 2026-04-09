#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

typedef struct Node Node;
typedef struct mem_context mem_context_t;

//! @brief Search a tree for a key
//! @param[in]  root  The root of the tree to search
//! @param[in]  key   The key to search for
//! @param[in]  ctx   Memory context (local + remote node table)
//! @return Struct containing requested data on success and an error code
bstatusval_t search(bptr_t root, bkey_t key, mem_context_t *ctx);

#endif
