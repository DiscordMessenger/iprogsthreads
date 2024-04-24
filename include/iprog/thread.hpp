//
//  iProgramInCpp's Thread Implementation for Windows
//
//  Copyright (C) 2024 iProgramInCpp.  Licensed under the MIT license.
//

#ifndef _IPROG_THREAD_
#define _IPROG_THREAD_

#include <functional>
#include <type_traits>
#include <tuple>
#include <memory>
#include <utility>
#include <chrono>
#include <system_error>

#include "w32constants.hpp"

#ifdef _DEBUG
extern void DbgPrintW(const char* fmt, ...);
#else
#define DbgPrintW(...)
#endif

namespace iprog {

class this_thread;

class thread
{
private:
	typedef void* native_handle_type;
	
#if __cplusplus > 201103L
	using idx_seq = std::index_sequence;
	using make_idx_seq = std::make_index_sequence;
	using create_unique_ptr = std::make_unique;
	template<typename T>
	using decay_type = std::decay_t<T>;
#else
	template<std::size_t...>
	struct idx_seq {};
	
	template<std::size_t n, std::size_t... s>
	struct make_idx_seq_aux : make_idx_seq_aux<n-1, n-1, s...> { };
	
	template<std::size_t... s>
	struct make_idx_seq_aux<0, s...> { typedef idx_seq<s...> type; };
	
	template<std::size_t n, std::size_t... s>
	using make_idx_seq = typename make_idx_seq_aux<n, s...>::type;
	
	template<typename T>
	using decay_type = typename std::decay<T>::type;
	
	template<typename T, typename... Args>
	std::unique_ptr<T> create_unique_ptr(Args&&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
#endif

public:
	class id {
	public:
		id() noexcept {
			m_id = 0;
		}
		bool operator==(const id& other) const noexcept {
			return m_id == other.m_id;
		}
		bool operator!=(const id& other) const noexcept {
			return m_id != other.m_id;
		}
		bool operator<(const id& other) const noexcept {
			return m_id < other.m_id;
		}
		bool operator<=(const id& other) const noexcept {
			return m_id <= other.m_id;
		}
		bool operator>(const id& other) const noexcept {
			return m_id > other.m_id;
		}
		bool operator>=(const id& other) const noexcept {
			return m_id >= other.m_id;
		}
	private:
		size_t m_id;
	protected:
		friend class iprog::thread;
		friend class iprog::this_thread;
		id(size_t id) noexcept {
			m_id = id;
		}
	};

public:
	// Creates a new thread object which does not represent a thread.
	thread() noexcept {
		m_handle = nullptr;
		m_id = 0;
	}

	// Destroys the thread object.  It may not have an associated thread.
	~thread() noexcept {
		if (joinable())
			std::terminate();
	}

	// Constructs the std::thread object to represent the thread of execution that
	// was represented by `other`. After this call, `other` no longer represents
	// a thread of execution.
	thread(thread&& other) noexcept {
		m_handle = other.m_handle;
		m_id = other.m_id;
		other.m_handle = nullptr;
		other.m_id = 0;
	}

	thread& operator=(thread&& other) noexcept {
		if (joinable())
			std::terminate();

		m_handle = other.m_handle;
		m_id = other.m_id;
		other.m_handle = nullptr;
		other.m_id = id(0);
		return (*this);
	}

	// Creates a new std::thread object and associates it with a thread of execution.
	template<class F, class... Args>
	explicit thread(F&& f, Args&&... args) {
		begin(f, args...);
	}

	// The copy constructor is deleted, as threads are not copyable.
	thread(const thread& other) = delete;

	id get_id() const noexcept {
		return id(m_id);
	}

	native_handle_type native_handle() const noexcept {
		return m_handle;
	}

	// Check whether the thread is joinable.
	bool joinable() const noexcept {
		return m_handle != nullptr;
	}

	// Separates this thread of execution from the thread object.
	void detach();

	// Blocks the current thread until the thread identified by `*this` finishes its execution.
	void join();

	// Exchanges the underlying handles of two thread objects.
	void swap(thread& other) noexcept {
		std::swap(m_handle, other.m_handle);
		std::swap(m_id, other.m_id);
	}
	
	static unsigned int hardware_concurrency() noexcept;
	
private:
	// Template based black magic.
	template<class Tuple, size_t... Indices>
	static void __stdcall invoke(void* params) noexcept {
		// This is the beginning function of the thread.
		const std::unique_ptr<Tuple> vals_ptr(static_cast<Tuple*>(params));
		Tuple& vals = *vals_ptr.get();
		// Note. I'd use std::invoke here, but that didn't come out until C++14. Bummer.
		std::bind(std::move(std::get<Indices>(vals))...)();
	}
	
	template<class Tuple, size_t... Indices>
	static constexpr void* get_invoker(idx_seq<Indices...>) noexcept {
		return (void*) &invoke<Tuple, Indices...>;
	}

	template <class F, class... Args>
	void begin(F&& f, Args&&... args) {
		using start_data_tuple = std::tuple<decay_type<F>, decay_type<Args>...>;
		auto decay_copied = create_unique_ptr<start_data_tuple>(std::forward<F>(f), std::forward<Args>(args)...);
		auto invoker_func = get_invoker<start_data_tuple>(make_idx_seq<1 + sizeof...(Args)>{});

		size_t tid = 0;
		m_handle = create_thread(invoker_func, decay_copied.get(), tid);
		m_id = id(tid);
		if (m_handle) {
			(void) decay_copied.release();
			return;
		}
		DbgPrintW("Resource Unavailable Try Again In THREAD::BEGIN");
		throw std::system_error(std::make_error_code(std::errc::resource_unavailable_try_again));
	}

	static native_handle_type create_thread(void* invokeptr, void* params, size_t& out_id) noexcept;

private:
	native_handle_type m_handle;
	id m_id;
};

class this_thread
{
public:
	static thread::id get_id() noexcept;
	
	template<class Rep, class Period>
	static void sleep_for(const std::chrono::duration<Rep, Period>& sleep_duration) {
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(sleep_duration).count();
		uint32_t sleep_time;
		if (millis < 0)
			sleep_time = 0;
		else if (millis >= __infinite)
			sleep_time = __infinite - 1;
		else
			sleep_time = (uint32_t) millis;
		
		perform_sleep(sleep_time);
	}
	
	template<class Clock, class Duration>
	static void sleep_until(const std::chrono::time_point<Clock, Duration>& sleep_time) {
		sleep_for(sleep_time - Clock::now());
	}
	
private:
	static void perform_sleep(uint32_t ms) noexcept;
};

} // namespace iprog

#endif//_IPROG_THREAD_