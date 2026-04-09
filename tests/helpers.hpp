#ifndef TESTS_HELPERS_HPP
#define TESTS_HELPERS_HPP


#include "../types.h"
#include <cstdio>


typedef struct Node Node;

//! @brief Argument to @ref stride_insert thread handler
struct si_args {
	//! First value to insert
	int_fast32_t start;
	//! Last value to insert,
	//! exact value may not be inserted if stride is misaligned
	int_fast32_t end;
	//! Difference between the current inserted value and the next inserted
	//! value
	int_fast32_t stride;
	//! Pointer to the tree root
	bptr_t *root;
	//! Set by the thread, read up on return.
	//! False if an insertion error has occurred, true otherwise.
	bool pass;
	FILE *log_stream;
};


extern Node memory[MEM_SIZE];
extern mem_context_t ctx;

//! @brief Thread function for inserting values over a range with a set interval
//! between them
//!
//! Can handle negative start, end, and stride values.
//! Invalid combinations of start, end, and stride will result in a test failure
//!
//! Keys are have values of 1 <= key <= n, Values are assigned to -1 times their
//! key
void *stride_insert(void *argv);
//! @brief Check leaf linked list correctncess after insertion of keys
//!
//! Keys are have values of 1 <= key <= n,
//! Values are assigned to -1 times their key
//! @return true if pass, false otherwise
bool check_inserted_leaves();


#endif
