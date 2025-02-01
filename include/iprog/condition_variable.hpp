//
//  iProgramInCpp's Thread Implementation for Windows
//
//  Copyright (C) 2024 iProgramInCpp.  Licensed under the MIT license.
//

#ifndef _IPROG_CONDITION_VARIABLE_
#define _IPROG_CONDITION_VARIABLE_

#include <atomic>
#include <chrono>
#include <system_error>

#include "w32constants.hpp"
#include "mutex.hpp"
#include "unique_lock.hpp"
#include "lock_guard.hpp"

// Thanks to https://github.com/meganz/mingw-std-threads for the help implementing this!

namespace iprog {

enum class cv_status {
	no_timeout,
	timeout,
};

class condition_variable_any
{
private:
	typedef void* native_handle;

public:
	condition_variable_any();

	condition_variable_any(const condition_variable_any&) = delete;

	~condition_variable_any();

	condition_variable_any& operator=(const condition_variable_any&) = delete;

	template<class M>
	void wait(M& lock) {
		wait_impl(lock, __infinite);
	}

	template<class M, class Predicate>
	void wait(M& lock, Predicate& pred) {
		while (!pred())
			wait(lock);
	}

	void notify_all() noexcept {
		lock_guard<recursive_mutex> lg(m_mutex);
		if (m_num_waiters.load() < 0)
			return;

		release_sem(m_semaphore, m_num_waiters, nullptr);
		
		while (m_num_waiters > 0) {
			auto ret = wait_object(m_wake_event, 1000);
			if (ret == __wait_failed || ret == __wait_abandoned)
				terminateIprogsThreads();
		}
		if (m_num_waiters != 0)
			terminateIprogsThreads();

		// In case some of the waiters timed out just after we released the
		// semaphore by m_num_waiters, it won't be zero now, because not all
		// the waiters woke up by acquiring the semaphore.  So we must zero
		// the semaphore before we accept waiters for the next event.
		while (wait_object(m_semaphore, 0) == __wait_object_0);
	}

	void notify_one() noexcept {
		notify_some(1);
	}

	template<class M, class Rep, class Per>
	cv_status wait_for(M& lock, const std::chrono::duration<Rep, Per>& rel_time) {
		auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(rel_time).count();
		uint32_t wait_time;
		if (timeout >= __infinite)
			wait_time = __infinite - 1;
		else if (timeout < 0)
			wait_time = 0;
		else
			wait_time = static_cast<uint32_t>(timeout);

		bool ret = wait_impl(lock, wait_time) || (timeout >= __infinite);
		return ret ? cv_status::no_timeout : cv_status::timeout;
	}

	template<class M, class Rep, class Per, class Pred>
	bool wait_for(M& lock, const std::chrono::duration<Rep, Per>& rel_time, Pred pred) {
		return wait_until(lock, std::chrono::steady_clock::now() + rel_time, pred);
	}

	template<class M, class Clock, class Duration>
	cv_status wait_until(M& lock, const std::chrono::time_point<Clock, Duration>& abs_time) {
		return wait_for(lock, abs_time - Clock::now());
	}
	
	template<class M, class Clock, class Duration, class Pred>
	bool wait_until(M& lock, const std::chrono::time_point<Clock, Duration>& abs_time, Pred pred) {
		while (!pred())
			if (wait_until(lock, abs_time) == cv_status::timeout)
				return pred();
		return true;
	}

private:
	template<class M>
	bool wait_impl(M& lock, uint32_t timeout) {
		m_mutex.lock();
		m_num_waiters++;
		m_mutex.unlock();
		lock.unlock();

		uint32_t ret = wait_object(m_semaphore, timeout);
		m_num_waiters--;
		set_event(m_wake_event);
		lock.lock();

		if (ret == __wait_object_0)
			return true;
		if (ret == __wait_timeout)
			return false;

		// 2 possible cases.
		// 1) The point in notify_all() where we determine the count to increment
		//    the semaphore by has not been reached yet, we just need to decrement
		//    m_num_waiters, but setting the event doesn't hurt
		// 2) Semaphore has just been released with m_num_waiters just before we
		//    decremented it.  This means that the semaphore count after all wait-
		//    ers finish won't be zero - because not all waiters woke up by acqui-
		//    ring the semaphore - we woke up by a timeout.	The notify_all() must
		//    handle this gracefully.

		throw std::system_error(std::make_error_code(std::errc::function_not_supported));
	}

	void notify_some(int count) noexcept {
		lock_guard<recursive_mutex> lg(m_mutex);
		int target_waiters = m_num_waiters - count;
		if (target_waiters < 0)
			return;

		release_sem(m_semaphore, count, nullptr);
		while (m_num_waiters > target_waiters) {
			auto ret = wait_object(m_wake_event, 1000);
			if (ret == __wait_failed || ret == __wait_abandoned)
				terminateIprogsThreads();
		}

		if (m_num_waiters != target_waiters)
			terminateIprogsThreads();
	}

	uint32_t wait_object(native_handle sem, uint32_t timeout);

	bool set_event(native_handle ev);

	bool release_sem(native_handle sem, long count, long* prev_count);

private:
	recursive_mutex m_mutex;

	std::atomic<long> m_num_waiters { 0 };

	native_handle m_semaphore = nullptr;

	native_handle m_wake_event = nullptr;
};

class condition_variable : public condition_variable_any
{
	using base = condition_variable_any;
	using ulm = unique_lock<mutex>;
public:

	void wait(ulm& lock) {
		base::wait(lock);
	}
	template<class P>
	void wait(ulm& lock, P p) {
		base::wait(lock, p);
	}
	template<class R, class P>
	cv_status wait_for(ulm& lock, const std::chrono::duration<R, P>& rt) {
		return base::wait_for(lock, rt);
	}
	template <class R, class P, class Pr>
	bool wait_for(unique_lock<mutex>& lock, const std::chrono::duration<R, P>& rt, Pr pr) {
		return base::wait_for(lock, rt, pr);
	}
	template <class C, class D>
	cv_status wait_until(unique_lock<mutex>& lock, const std::chrono::time_point<C, D>& at) {
		return base::wait_until(lock, at);
	}
	template <class C, class D, class Pr>
	bool wait_until(unique_lock<mutex>& lock, const std::chrono::time_point<C, D>& at, Pr pr) {
		return base::wait_until(lock, at, pr);
	}
};

} // namespace iprog

#endif//_IPROG_CONDITION_VARIABLE_