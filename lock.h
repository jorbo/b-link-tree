#ifndef LOCK_H
#define LOCK_H

#include "defs.h"
#include <stdbool.h>

/* pthread / assert / string are not available in HLS synthesis */
#ifndef __SYNTHESIS__
#include <pthread.h>
#include <assert.h>
#include <string.h>
#endif

#ifdef OPTIMISTIC_LOCK
	#include <stdint.h>
	typedef uint_fast32_t lock_t;
	#define LOCK_INIT 0
#else
#if defined(__SYNTHESIS__)
	#include <stdint.h>
	typedef uint32_t lock_t;
	#define LOCK_INIT 0
#else
	typedef pthread_mutex_t lock_t;
	#define LOCK_INIT ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)
#endif
#endif


#ifdef OPTIMISTIC_LOCK
static inline bool lock_test(lock_t const *lock, lock_t const *lock_ref) {
	return *lock != *lock_ref;
}
#else
static inline bool lock_test(lock_t const *lock) {
	#ifndef UNLOCKED
		#if defined(__SYNTHESIS__)
			return *lock != LOCK_INIT;
		#else
			lock_t unset = LOCK_INIT;
			//! @warning This should work in most cases, but allegedly
			//! technically if the system's lock type is padded, then
			//! theoretically the padding may not be equal and cause an
			//! incorrect return of true
			return memcmp(lock, &unset, sizeof(lock_t));
		#endif
	#else
		return false;
	#endif
}
#endif

#ifndef OPTIMISTIC_LOCK
//! @brief Perform an atomic test-and-set operation on the given lock
static inline bool test_and_set(lock_t *lock) {
	#ifndef UNLOCKED
		#if defined(__SYNTHESIS__)
			bool old = *lock;
			*lock = true;
			return old;
		#else
			return pthread_mutex_trylock(lock);
		#endif
	#else
		return lock_test(lock);
	#endif
}


//! @brief Set the given lock to held
static inline void lock_p(lock_t *lock) {
	#ifndef UNLOCKED
		#if defined(__SYNTHESIS__)
			while (test_and_set(lock));
		#else
			pthread_mutex_lock(lock);
		#endif
	#endif
}

//! @brief Release the given lock
static inline void lock_v(lock_t *lock) {
	#ifndef UNLOCKED
		#if defined(__SYNTHESIS__)
			*lock = 0;
		#else
			assert(pthread_mutex_unlock(lock) == 0);
		#endif
	#endif
}
#endif

#endif
