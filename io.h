#ifndef IO_H
#define IO_H

#include "node.h"
#include <stdio.h>

//! @brief Print the contents of a tree's memory in a human-readable format
//! @param[out] stream  Output stream to write to, can be a file or standard
//!                     output
void dump_node_list(FILE *stream, Node const *memory);

//! @brief Print the contents of the tree as a DOT file that can be read with
//!        Graphviz
//! @param[out] stream  Output stream to write to, can be a file or standard
//!                     output
void dump_gv(FILE *stream, Node const *memory);

#endif
