//
//  iProgramInCpp's Thread Implementation for Windows
//
//  Copyright (C) 2024 iProgramInCpp.  Licensed under the MIT license.
//

#include <iprog/thread.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef  USE_IPROGS_REIMPL
#	include <ri/reimpl.hpp>
#else
#	include <process.h>
#endif

namespace iprog {

thread::native_handle_type thread::create_thread(void* invokeptr, void* params, size_t& out_id) noexcept
{
	// Create the actual thread.
	unsigned threadId = 0;
	HANDLE hnd = (HANDLE) _beginthreadex(NULL, 0, (_beginthreadex_proc_type)invokeptr, params, 0, &threadId);

	// Assign the output thread ID.
	if (!hnd) threadId = 0;
	out_id = (size_t) threadId;

	// Return the handle to the thread.
	return (native_handle_type) hnd;
}

void thread::join()
{
	if (!joinable()) {
		DbgPrintW("invalid_argument in thread::join");
		throw std::system_error(std::make_error_code(std::errc::invalid_argument));
	}

	// TODO: check against this thread

	// Wait for the thread to exit.
	WaitForSingleObject(m_handle, INFINITE);

	// The thread has now exited.  Reset its properties.
	CloseHandle(m_handle);
	m_handle = 0;
	m_id = id();
}

void thread::detach()
{
	if (!joinable()) {
		DbgPrintW("invalid_argument in thread::detach");
		throw std::system_error(std::make_error_code(std::errc::invalid_argument));
	}

	// TODO: check against this thread

	// Close the handle to the thread.  It will keep running.
	CloseHandle(m_handle);
	m_handle = 0;
	m_id = id();
}

unsigned int thread::hardware_concurrency() noexcept
{
	// TODO
	return 4;
}

thread::id this_thread::get_id() noexcept
{
	return thread::id(GetCurrentThreadId());
}

void this_thread::perform_sleep(uint32_t ms) noexcept
{
	Sleep((DWORD) ms);
}

} // namespace iprog
