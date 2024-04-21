#ifndef _IPROG_MUTEX_
#define _IPROG_MUTEX_

#include "recursive_mutex.hpp"

// Implement mutex on top of recursive_mutex.

namespace iprog {

class mutex
{
public:
	mutex() noexcept;

	mutex(const mutex&) = delete;

	~mutex() noexcept;

	mutex& operator=(const mutex&) = delete;

	void lock();

	bool try_lock();

	// TODO: try_lock_for
	// TODO: try_lock_until

	void unlock();

private:
	recursive_mutex m_driver;

	bool m_owned = false;
};

} // namespace iprog

#endif//_IPROG_MUTEX_