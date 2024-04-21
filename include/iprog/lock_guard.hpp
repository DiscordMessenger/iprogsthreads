//
//  iProgramInCpp's Thread Implementation for Windows
//
//  Copyright (C) 2024 iProgramInCpp.  Licensed under the MIT license.
//

#ifndef _IPROG_LOCK_GUARD_
#define _IPROG_LOCK_GUARD_

#include "lock_tags.hpp"

namespace iprog {

template<class Mutex>
class lock_guard
{
public:
	explicit lock_guard(Mutex& m) : m_mutex(m) {
		m_mutex.lock();
	}
	lock_guard(Mutex& m, const adopt_lock_t&) {
		m_mutex = m;
		// Assume the caller has ownership of this mutex already.
	}
	~lock_guard() noexcept {
		m_mutex.unlock();
	}
	lock_guard(const lock_guard&) = delete;
	lock_guard& operator=(const lock_guard&) = delete;

private:
	Mutex& m_mutex;
};

} // namespace iprog

#endif//_IPROG_LOCK_GUARD_