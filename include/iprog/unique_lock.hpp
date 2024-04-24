//
//  iProgramInCpp's Thread Implementation for Windows
//
//  Copyright (C) 2024 iProgramInCpp.  Licensed under the MIT license.
//

#ifndef _IPROG_UNIQUE_LOCK_
#define _IPROG_UNIQUE_LOCK_

#include <exception>
#include <system_error>

#include "lock_tags.hpp"

namespace iprog {

template<class Mutex>
class unique_lock
{
public:
	unique_lock() noexcept {
		m_mutex = nullptr;
		m_owns = false;
	}

	explicit unique_lock(Mutex& m) {
		m_mutex = &m;
		lock();
	}

	unique_lock(Mutex& m, defer_lock_t t) noexcept {
		m_mutex = &m;
		m_owns = false;
	}

	unique_lock(Mutex& m, adopt_lock_t t) noexcept {
		m_mutex = &m;
		m_owns = true;
	}

	unique_lock(Mutex& m, try_to_lock_t t) {
		m_mutex = &m;
		m_owns = m_mutex->try_lock();
	}

	unique_lock(const unique_lock&) = delete;

	unique_lock(unique_lock&& oth) {
		m_mutex = oth.m_mutex;
		m_owns  = oth.m_owns;
		oth.m_mutex = nullptr;
		oth.m_owns  = false;
	}

	~unique_lock() {
		if (m_owns)
			unlock();
	}
	
	unique_lock& operator=(unique_lock&& oth) noexcept {
		if (m_owns)
			unlock();
		m_mutex = oth.m_mutex;
		m_owns  = oth.m_owns;
		oth.m_mutex = nullptr;
		oth.m_owns  = false;
		return (*this);
	}

	void lock() {
		if (!m_mutex) {
			throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
			return;
		}
		if (m_owns) {
			throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
			return;
		}
		m_mutex->lock();
		m_owns = true;
	}

	bool try_lock() {
		if (!m_mutex) {
			throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
			return false;
		}
		if (m_owns) {
			throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
			return false;
		}
		m_owns = m_mutex->try_lock();
		return m_owns;
	}

	// TODO: try_lock_for
	// TODO: try_lock_until

	void unlock() {
		if (!m_owns) {
			throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
			return;
		}
		m_mutex->unlock();
		m_owns = false;
	}

	void swap(unique_lock& other) noexcept {
		std::swap(m_mutex, other.m_mutex);
		std::swap(m_owns, other.m_owns);
	}

	Mutex* release() noexcept {
		Mutex* temp = m_mutex;
		m_mutex = nullptr;
		m_owns = false;
		return temp;
	}

	bool owns_lock() const noexcept {
		return m_owns;
	}

	explicit operator bool() const noexcept {
		return owns_lock();
	}

	Mutex* mutex() {
		return m_mutex;
	}

private:
	Mutex* m_mutex = nullptr;
	bool m_owns = false;
};

template<class Mutex>
void swap(unique_lock<Mutex>& a, unique_lock<Mutex>& b) {
	a.swap(b);
}

} // namespace iprog

#endif//_IPROG_UNIQUE_LOCK_