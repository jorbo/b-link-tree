#include "validate.h"
#include "memory.h"
#include "node.h"
#include "tree-helpers.h"


static li_t num_children(bptr_t node, mem_context_t *ctx) {
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (mem_read(node, ctx).keys[i] == INVALID)
			return i;
	}
	return TREE_ORDER;
}


//! @return `true` for passing, `false` for failing
static bool validate_root(bptr_t root, FILE *stream, mem_context_t *ctx) {
	li_t n_child = num_children(root, ctx);
	bool result = is_leaf(root) || n_child >= 2;
	fprintf(stream, "Validating mem[%u] (root)...", root);
	if (result) {
		fprintf(stream, "valid!\n");
	} else {
		fprintf(stream, "invalid,\n\t");
		fprintf(stream,
			"root is not a leaf and has too few children (%u < %u)\n",
			n_child, 2
		);
	}
	return result;
}

//! @return `true` for passing, `false` for failing
static bool validate_node(bptr_t root, bptr_t node, FILE *stream, mem_context_t *ctx) {
	if (node == root) {
		return validate_root(root, stream, ctx);
	} else {
		fprintf(stream, "mem[%u]...", node);
		if (bptr_local_addr(node) >= MEM_SIZE) {
			fprintf(stream, "invalid, local address %u >= %u\n",
				bptr_local_addr(node), MEM_SIZE);
			return false;
		}
		li_t n_child = num_children(node, ctx);
		bool result = is_leaf(node) || n_child >= TREE_ORDER / 2;
		if (result) {
			fprintf(stream, "valid!\n");
		} else {
			fprintf(stream, "invalid,\n\t");
			fprintf(stream,
				"node is not a leaf and has too few children (%u < %u)\n",
				n_child, TREE_ORDER / 2
			);
		}
		return result;
	}
}

static bool validate_children(bptr_t root, bptr_t node, FILE *stream, mem_context_t *ctx) {
	bool result = true;
	if (!validate_node(root, node, stream, ctx)) {
		result = false;
	}
	if (!is_leaf(node)) {
		fprintf(stream, "Validating mem[%u]'s children...\n", node);
		for (li_t i = 0; i < TREE_ORDER; ++i) {
			Node n = mem_read(node, ctx);
			fprintf(stream, "Validating child %u, ", i);
			if (n.keys[i] == INVALID) {
				fprintf(stream, "out of children\n");
				break;
			} else if (bptr_local_addr(n.values[i].ptr) >= MEM_SIZE) {
				fprintf(stream,
					"invalid, local address %u >= %u\n",
					bptr_local_addr(n.values[i].ptr), MEM_SIZE
				);
				result = false;
			} else if (!validate_children(root, n.values[i].ptr, stream, ctx)) {
				result = false;
			}
		}
	}
	fflush(stream);
	return result;
}

bool validate(bptr_t root, FILE *stream, mem_context_t *ctx) {
	return validate_children(root, root, stream, ctx);
}


//! @return `true` if `node` and all of its children are unlocked,
//!         `false` otherwise
static bool subtree_unlocked(bptr_t node, FILE *stream, mem_context_t *ctx) {
#ifdef OPTIMISTIC_LOCK
	return true;
#else
	Node n = mem_read(node, ctx);
	bool result = !lock_test(&n.lock);

	fprintf(stream, "Checking mem[%u]'s children...\n", node);

	if (is_leaf(node)) {
		if (!result) fprintf(stream, "mem[%u] is still locked!\n", node);
	} else {
		for (li_t i = 0; i < TREE_ORDER; ++i) {
			if (n.keys[i] == INVALID) return result;
			result |= subtree_unlocked(n.values[i].ptr, stream, ctx);
		}
	}

	return result;
#endif
}

//! @return `true` if all nodes in this tree are unlocked, `false` otherwise
bool is_unlocked(bptr_t root, FILE *stream, mem_context_t *ctx) {
	return subtree_unlocked(root, stream, ctx);
}
