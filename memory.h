#ifndef MEMORY_H
#define MEMORY_H


#include "node.h"
#include "types.h"
#include <stddef.h>
#include <stdbool.h>


//! @brief Read a node from memory without grabbing any locks
Node mem_read(bptr_t address, Node const **memory);

//! @brief Read a node from memory, locking it in memory
Node mem_read_lock(bptr_t address, Node **memory);

//! @brief Read a node from memory, locking it in memory if possible,
//         otherwise setting success to false
Node mem_read_trylock(bptr_t address, Node **memory, bool *success);


#ifdef OPTIMISTIC_LOCK
#include <stdbool.h>
//! @brief Write a node to memory and unlock it
//! @return true on success, false on failure
bool mem_write_unlock(AddrNode *node, Node **memory);
#else
//! @brief Write a node to memory and unlock it
void mem_write_unlock(AddrNode *node, Node **memory);
#endif

//! @brief Unlock a node in memory
//!
//! Used when a node has been read and locked, but should be unlocked without
//! modification. For example, if an error has been detected before a
//! modification operation completes.
void mem_unlock(bptr_t address, Node **memory);

//! @brief Reset memory to a slate of blank nodes
//!
//! All data is 1s except for locks
void mem_reset_all(Node **memory);


#endif
