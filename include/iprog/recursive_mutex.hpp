//
//  iProgramInCpp's Thread Implementation for Windows
//
//  Copyright (C) 2024 iProgramInCpp.  Licensed under the MIT license.
//

#ifndef _IPROG_RECURSIVE_MUTEX_
#define _IPROG_RECURSIVE_MUTEX_

namespace iprog {

class recursive_mutex
{
private:
	typedef void* native_handle_type;

public:
	recursive_mutex() noexcept;

	recursive_mutex(const recursive_mutex&) = delete;

	~recursive_mutex() noexcept;

	recursive_mutex& operator=(const recursive_mutex&) = delete;

	void lock();

	bool try_lock();

	// TODO: try_lock_for
	// TODO: try_lock_until

	void unlock();

private:
	native_handle_type m_handle;
};

} // namespace iprog

#endif//_IPROG_MUTEX_