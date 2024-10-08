#pragma once
#include <cdefs.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Returns true if the trusted stack contains at least `requiredFrames` frames
 * past the current one, false otherwise.
 *
 * Note: This is faster than calling either `trusted_stack_index` or
 * `trusted_stack_size` and so should be preferred in guards.
 */
__cheri_libcall _Bool trusted_stack_has_space(int requiredFrames);

/**
 * Recover the stack value that was passed into a compartment on
 * cross-compartment call.  This allows sub-compartment compartmentalization
 * (e.g. running another OS in a compartment) where the monitor in the
 * compartment uses heap-allocated stacks for the nested compartments, while
 * providing a mechanism for the monitor to recover the stack.
 */
__cheri_libcall void *switcher_recover_stack(void);

/**
 * Returns a sealed capability to the current thread.  This can be used as the
 * argument to `switcher_interrupt_thread` from another thread to interrupt
 * this thread (invoking its error handler) if and only if the other thread is
 * currently executing in this compartment.  This can be used to implement
 * watchdog events from another thread.
 */
__cheri_libcall void *switcher_current_thread(void);

/**
 * Interrupt given thread (invoke its error handler). This can be used with the
 * value returned by `switcher_current thread` in another thread to interrupt
 * that thread if and only if the other thread is currently executing in the
 * same compartment as this thread. This can be used to implement watchdog
 * events from another thread. An attempt to interrupt the current thread will
 * return failure.
 *
 * Returns true on success, false on failure.
 *
 * This should typically be followed by a yielding operation.  The interrupted
 * thread is not woken until the scheduler runs.
 */
__cheri_libcall _Bool switcher_interrupt_thread(void *);

/**
 * Returns a store-only capability to two hazard pointer slots for the current
 * thread.  Objects stored here will not be deallocated until (at least) the
 * next cross-compartment call or until they are explicitly overwritten.
 */
__cheri_libcall void **switcher_thread_hazard_slots(void);

/**
 * Returns the lowest address that has been stored to on the stack in this
 * compartment invocation.
 */
__cheri_libcall ptraddr_t stack_lowest_used_address(void);

/**
 * Resets the switcher's count of invocations.
 *
 * Switcher place a limit on the number of times a compartment invocation may
 * fault (default 512).  This prevents a compartment from getting stuck
 * partially recovering from errors.  This function resets the count to zero.
 * It should be called from outer compartments' run loops and from places where
 * the caller is certain that error handling is making forward progress.
 *
 * Note: If this is called from an error handler that subsequently returns with
 * with install-context, the switcher will subtract one from this and set the
 * switcher error count to -1.  Because this is odd, the switcher will detect
 * the next invocation of the error handler as a double fault and will force
 * unwind.  Do not call this if you are currently in an error handler unless
 * you are jumping out via some mechanism that does *not* involve returning to
 * the switcher.
 *
 * Returns the previous value of the invocation count.  The low bit is set if
 * an error handler is currently running, the remaining bits are the count.
 */
__cheri_libcall uint16_t switcher_handler_invocation_count_reset(void);

