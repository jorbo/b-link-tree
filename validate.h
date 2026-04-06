#ifndef VALIDATE_H
#define VALIDATE_H


#include "types.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct Node Node;


//! @brief Check the correctness of the underlying tree assumptions
//! @param[in]  root    Root of the tree to check
//! @param[out] stream  An output stream to write the validation output to
//! @return `true` for passing, `false` for failing
bool validate(bptr_t root, FILE *stream, Node const **memory);

//! @brief Check whether all nodes of a tree are unlocked,
//!        should return true when tree is at idle
//! @param[in]  root    Root of the tree to check
//! @param[out] stream  An output stream to write the validation output to
//! @return `true` if all nodes are unlocked, `false` otherwise
bool is_unlocked(bptr_t root, FILE *stream, Node const **memory);


#endif
