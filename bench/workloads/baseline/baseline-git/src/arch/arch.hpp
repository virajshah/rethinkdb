#ifndef __ARCH_ARCH_HPP__
#define __ARCH_ARCH_HPP__

/* Select platform-specific stuff */

/* #if WE_ARE_ON_LINUX */

#include "arch/linux/arch.hpp"
typedef linux_io_config_t platform_io_config_t;

/* #elif WE_ARE_ON_WINDOWS

#include "arch/win32/arch.hpp"
typedef win32_io_config_t platform_io_config_t

#elif ...

...

#endif */

/* Optionally mock the IO layer */

#ifndef MOCK_IO_LAYER

typedef platform_io_config_t io_config_t;

#else

#include "arch/mock/arch.hpp"
typedef mock_io_config_t<platform_io_config_t> io_config_t;

#endif

/* Move stuff into global namespace */

typedef io_config_t::thread_pool_t thread_pool_t;

typedef io_config_t::direct_file_t direct_file_t;
typedef io_config_t::iocallback_t iocallback_t;

typedef io_config_t::net_listener_t net_listener_t;
typedef io_config_t::net_listener_callback_t net_listener_callback_t;

typedef io_config_t::net_conn_t net_conn_t;
typedef io_config_t::net_conn_read_external_callback_t net_conn_read_external_callback_t;
typedef io_config_t::net_conn_read_buffered_callback_t net_conn_read_buffered_callback_t;
typedef io_config_t::net_conn_write_external_callback_t net_conn_write_external_callback_t;

typedef io_config_t::oldstyle_net_conn_t oldstyle_net_conn_t;
typedef io_config_t::oldstyle_net_conn_callback_t oldstyle_net_conn_callback_t;

typedef io_config_t::thread_message_t thread_message_t;

inline int get_thread_id() {
    return io_config_t::get_thread_id();
}

inline int get_num_threads() {
    return io_config_t::get_num_threads();
}

// continue_on_thread() is used to send a message to another thread. If the 'thread' parameter is the
// thread that we are already on, then it returns 'true'; otherwise, it will cause the other
// thread's event loop to call msg->on_thread_switch().
inline bool continue_on_thread(int thread, thread_message_t *msg) {
    return io_config_t::continue_on_thread(thread, msg);
}

// call_later_on_this_thread() will cause msg->on_thread_switch() to be called from the main event loop
// of the thread we are currently on. It's a bit of a hack.
inline void call_later_on_this_thread(thread_message_t *msg) {
    return io_config_t::call_later_on_this_thread(msg);
}

/* TODO: It is common in the codebase right now to have code like this:

if (continue_on_thread(thread, msg)) call_later_on_this_thread(msg);

This is because originally clients would just call store_message() directly.
When continue_on_thread() was written, the code still assumed that the message's
callback would not be called before continue_on_thread() returned. Using
call_later_on_this_thread() is not ideal because it would be better to just
continue processing immediately if we are already on the correct thread, but
at the time it didn't seem worth rewriting it, so call_later_on_this_thread()
was added to make it easy to simulate the old semantics. */

typedef io_config_t::timer_token_t timer_token_t;

inline timer_token_t *add_timer(long ms, void (*callback)(void *), void *ctx) {
    return io_config_t::add_timer(ms, callback, ctx);
}
inline timer_token_t *fire_timer_once(long ms, void (*callback)(void *), void *ctx) {
    return io_config_t::fire_timer_once(ms, callback, ctx);
}
inline void cancel_timer(timer_token_t *timer) {
    io_config_t::cancel_timer(timer);
}

#endif /* __ARCH_ARCH_HPP__ */