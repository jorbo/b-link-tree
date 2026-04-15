#ifndef LOCK_H
#define LOCK_H

#include "defs.h"
#include <stdbool.h>
#include <stdint.h>

/* Use a fixed-width type so sizeof(Node) is identical in csim, synthesis,
   and host builds — mismatched sizes corrupt cosim memory transfers. */
#ifdef OPTIMISTIC_LOCK
	typedef uint32_t lock_t;
	#define LOCK_INIT 0
#else
	typedef uint32_t lock_t;
	#define LOCK_INIT 0
#endif

/* assert is only available on the host */
#ifndef __SYNTHESIS__
#include <assert.h>
#endif


#ifdef OPTIMISTIC_LOCK
static inline bool lock_test(lock_t const *lock, lock_t const *lock_ref) {
	return *lock != *lock_ref;
}
#else
static inline bool lock_test(lock_t const *lock) {
	#ifndef UNLOCKED
		return *lock != LOCK_INIT;
	#else
		return false;
	#endif
}
#endif

#ifndef OPTIMISTIC_LOCK
//! @brief Perform an atomic test-and-set operation on the given lock
static inline bool test_and_set(lock_t *lock) {
	#ifndef UNLOCKED
		bool old = (*lock != 0);
		*lock = 1;
		return old;
	#else
		return lock_test(lock);
	#endif
}

//! @brief Set the given lock to held (spin until acquired)
static inline void lock_p(lock_t *lock) {
	#ifndef UNLOCKED
		while (test_and_set(lock));
	#endif
}

//! @brief Release the given lock
static inline void lock_v(lock_t *lock) {
	#ifndef UNLOCKED
		*lock = 0;
	#endif
}
#endif

#endif
