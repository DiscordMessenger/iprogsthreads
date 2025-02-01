//
//  iProgramInCpp's Thread Implementation for Windows
//
//  Copyright (C) 2024 iProgramInCpp.  Licensed under the MIT license.
//

#ifndef _IPROG_CALL_ONCE_
#define _IPROG_CALL_ONCE_

#include <atomic>

#include "override_terminate.hpp"
#include "mutex.hpp"
#include "lock_guard.hpp"

namespace iprog {

class once_flag
{
public:
	once_flag() noexcept {}
	once_flag(const once_flag&) = delete;
	once_flag& operator=(const once_flag&) = delete;

	template<class Callable, class... Args>
	friend void call_once(once_flag& once, Callable&& f, Args&&... args);

protected:
	std::atomic<bool> m_run{ false };
	mutex m_mtx;
};

template<class Callable, class... Args>
void call_once(once_flag& once, Callable&& f, Args&&... args) {
	if (once.m_run.load(std::memory_order_acquire))
		return;

	lock_guard<mutex> lg(once.m_mtx);
	if (once.m_run.load(std::memory_order_relaxed))
		return;

	once.m_run.store(true, std::memory_order_release);
	std::bind(std::forward<Callable>(f), std::forward<Args>(args)...)();
}

};

#endif//_IPROG_CALL_ONCE_
