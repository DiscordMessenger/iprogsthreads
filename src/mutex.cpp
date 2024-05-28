//
//  iProgramInCpp's Thread Implementation for Windows
//
//  Copyright (C) 2024 iProgramInCpp.  Licensed under the MIT license.
//

#include <iprog/thread.hpp>
#include <iprog/mutex.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace iprog {

recursive_mutex::recursive_mutex() noexcept
{
	LPCRITICAL_SECTION crit = new CRITICAL_SECTION;
	InitializeCriticalSection(crit);
	m_handle = crit;
}

recursive_mutex::~recursive_mutex() noexcept
{
	LPCRITICAL_SECTION crit = (LPCRITICAL_SECTION) m_handle;
	DeleteCriticalSection(crit);
	delete crit;
}

void recursive_mutex::lock()
{
	LPCRITICAL_SECTION crit = (LPCRITICAL_SECTION) m_handle;
	EnterCriticalSection(crit);
}

bool recursive_mutex::try_lock()
{
	LPCRITICAL_SECTION crit = (LPCRITICAL_SECTION) m_handle;
	return TryEnterCriticalSection(crit) == TRUE;
}

void recursive_mutex::unlock()
{
	LPCRITICAL_SECTION crit = (LPCRITICAL_SECTION) m_handle;
	LeaveCriticalSection(crit);
}

mutex::mutex() noexcept
{
}

mutex::~mutex() noexcept
{
}

void mutex::lock()
{
	m_driver.lock();

	// N.B. If m_driver is owned by another thread, it wouldn't even get here.  The case where
	// we reach here with a locked driver recursive mutex, and m_owned is true, is when the same
	// thread attempts to lock a mutex again.
	if (m_owned) {
		m_driver.unlock();
#ifdef _DEBUG
		OutputDebugStringA("iprogsthread warning: resource_deadlock_would_occur in mutex::lock\n");
#endif
		throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
		return;
	}

	m_owned = true;
}

bool mutex::try_lock()
{
	bool result = m_driver.try_lock();

	if (!result)
		return result;

	// A lock was successful, check if we already owned it. Again, the only thread that can
	// get here with m_owned == true, is the actual thread that locked it the first time.
	if (m_owned) {
		m_driver.unlock();
		DbgPrintW("resource_deadlock_would_occur In mutex::try_lock");
		throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
		return false;
	}

	m_owned = true;
	return true;
}

void mutex::unlock()
{
	// N.B. This is fine as long as the mutex isn't already unlocked when this is called.
	m_owned = false;
	m_driver.unlock();
}

} // namespace iprog
