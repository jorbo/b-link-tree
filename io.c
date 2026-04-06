#include "io.h"
#include "lock.h"
#include "memory.h"
#include "node.h"
#include <stdio.h>

//! @brief Print keys of a node in a human-readable format
//!
//! Helper function for @ref dump_node_list
//! @param[out] stream  Output stream to write to, can be a file or standard
//!                     output
//! @param[in]  node    The node whose keys to display
void dump_keys(FILE *stream, Node const *node) {
	fprintf(stream, "[");
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (node->keys[i] == INVALID) {
			fprintf(stream, "   ");
		} else {
			fprintf(stream, "%3u", node->keys[i]);
		}
		if (i < TREE_ORDER-1) {
			fprintf(stream, ", ");
		}
	}
	#ifdef OPTIMISTIC_LOCK
	fprintf(stream, "; %3u] ", node->lock);
	#else
	if (lock_test(&node->lock)) {
		fprintf(stream, "; LCK] ");
	} else {
		fprintf(stream, ";    ] ");
	}
	#endif
}

//! @brief Print values of a node in a human-readable format
//!
//! Helper function for @ref dump_node_list
//! @param[out] stream  Output stream to write to, can be a file or standard
//!                     output
//! @param[in]  node    The node whose values to display
void dump_values(FILE *stream, Node const *node) {
	fprintf(stream, "{");
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (node->keys[i] == INVALID) {
			fprintf(stream, "   ");
		} else {
			fprintf(stream, "%3d", node->values[i].data);
		}
		if (i < TREE_ORDER-1) {
			fprintf(stream, ", ");
		}
	}
	if (node->next == INVALID) {
		fprintf(stream, ";    ");
	} else {
		fprintf(stream, "; %3u", node->next);
	}
	fprintf(stream, "} ");
}


void dump_node_list(FILE *stream, Node const **memory) {
	Node n;
	uint_fast16_t i, r, c;
	fprintf(stream, "LEAVES\n%2u ", 0);
	for (i = 0; i < MAX_LEAVES; ++i) {
		n = memory[0][i];
		dump_keys(stream, &n);
	}
	fprintf(stream, "\n   ");
	for (i = 0; i < MAX_LEAVES; ++i) {
		n = memory[0][i];
		dump_values(stream, &n);
	}
	fprintf(stream, "\n");
	fprintf(stream, "INTERNAL NODES\n");
	for (r = 1; r < (MAX_LEVELS-1); ++r) {
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wformat"
		fprintf(stream, "%2u ", r*MAX_NODES_PER_LEVEL);
		#pragma GCC diagnostic pop
		for (c = 0; c < MAX_NODES_PER_LEVEL; ++c) {
			n = memory[r][c];
			dump_keys(stream, &n);
		}
		fprintf(stream, "\n   ");
		for (c = 0; c < MAX_NODES_PER_LEVEL; ++c) {
			n = memory[r][c];
			dump_values(stream, &n);
		}
		fprintf(stream, "\n");
	}
	fprintf(stream, "\n");
	fflush(stream);
}

void dump_gv(FILE *stream, Node const **memory) {
	fprintf(stream, "digraph g {\n");
	fprintf(stream, "\tnode [shape=record];\n");
	for (bptr_t level = 0; level < MAX_LEVELS; ++level) {
		for (bptr_t col = 0; col < MAX_NODES_PER_LEVEL; ++col) {
			Node const *n = &memory[level][col];
			bptr_t i = bptr_make(level, col);
			if (n->keys[0] == INVALID) continue;
			// Nodes
			fprintf(stream, "\tnode%d[label = \"", i);
			for (li_t j = 0; j < TREE_ORDER; ++j) {
				if (n->keys[j] == INVALID) {
					fprintf(stream, "{|}|");
				} else {
					fprintf(stream, "{%d|<k%d>0x%x}|",
						n->keys[j], j, n->values[j]);
				}
			}
			if (n->next != INVALID) {
				fprintf(stream, "{<k%d>0x%x|%s}\"];\n",
					TREE_ORDER, n->next,
					lock_test(&n->lock) ? "LCK" : "   ");
				fprintf(stream, "\t{rank=same;\"node%d\":k%d->\"node%d\"}\n",
					i, TREE_ORDER, n->next);
			} else {
				fprintf(stream, "{|%s}\"];\n",
					lock_test(&n->lock) ? "LCK" : "");
			}
			// Links
			if (!is_leaf(i)) {
				for (li_t j = 0; j < TREE_ORDER; ++j) {
					if (n->keys[j] == INVALID) {
						break;
					} else {
						fprintf(stream, "\t\"node%d\":k%d->\"node%d\"\n",
							i, j, n->values[j]);
					}
				}
			}
		}
	}
	fprintf(stream, "}\n");
	fflush(stream);
}
