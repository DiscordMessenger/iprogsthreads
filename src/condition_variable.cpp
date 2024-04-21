//
//  iProgramInCpp's Thread Implementation for Windows
//
//  Copyright (C) 2024 iProgramInCpp.  Licensed under the MIT license.
//

#include <iprog/condition_variable.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace iprog {

condition_variable_any::condition_variable_any()
{
	m_semaphore = (native_handle) CreateSemaphore(NULL, 0, 0xFFFF, NULL);
	if (!m_semaphore) {
		throw std::system_error(std::make_error_code(std::errc::resource_unavailable_try_again));
	}

	m_wake_event = (native_handle) CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!m_wake_event) {
		CloseHandle(m_semaphore);
		m_semaphore = nullptr;
		throw std::system_error(std::make_error_code(std::errc::resource_unavailable_try_again));
	}
}

condition_variable_any::~condition_variable_any()
{
	if (m_semaphore)
		CloseHandle(m_semaphore);
	if (m_wake_event)
		CloseHandle(m_wake_event);
}

uint32_t condition_variable_any::wait_object(native_handle sem, uint32_t timeout)
{
	DWORD res = WaitForSingleObject((HANDLE) sem, (DWORD) timeout);
	return (uint32_t) res;
}

bool condition_variable_any::set_event(native_handle ev)
{
	return SetEvent((HANDLE) ev) != FALSE;
}

bool condition_variable_any::release_sem(native_handle sem, long count, long* prev_count)
{
	return ReleaseSemaphore((HANDLE) sem, (LONG) count, (LPLONG) prev_count) != FALSE;
}

} // namespace iprog
