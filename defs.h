#ifndef DEFS_H
#define DEFS_H

//! Number of children in each node of a tree
#ifndef TREE_ORDER
#define TREE_ORDER (4)
#endif
//! "Width" of a memory grid
//! Each level of the tree is on its own level
#ifndef MAX_NODES_PER_LEVEL
#define MAX_NODES_PER_LEVEL (10)
#endif
//! Maximum height of a tree
#ifndef MAX_LEVELS
#define MAX_LEVELS (4)
#endif
//! Maximum number of leaf nodes in the entire tree
#ifndef MAX_LEAVES
#define MAX_LEAVES MAX_NODES_PER_LEVEL
#endif
//! Maximum number of nodes that can be stored in a tree's memory
#ifndef MEM_SIZE
#define MEM_SIZE (MAX_NODES_PER_LEVEL * MAX_LEVELS)
#endif

#if !defined NO_GTEST && !__has_include("gtest/gtest.h")
#define NO_GTEST
#endif

#ifndef RDMA_LANDING_SLOTS
#define RDMA_LANDING_SLOTS 2
#endif

#endif
