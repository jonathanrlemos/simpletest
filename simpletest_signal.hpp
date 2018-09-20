/** @file simpletest_signal.hpp
 * @brief simpletest signal handler.
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __SIMPLETEST_SIGNAL_HPP
#define __SIMPLETEST_SIGNAL_HPP

#include <setjmp.h>
#include <csignal>
#include <mutex>

namespace simpletest{

/**
 * @brief This singleton globally manages signal handling.
 * All operations in this class are thread-safe.
 */
class SignalHandler{
public:
	/**
	 * @brief Gets the global instance of the signal handler.
	 */
	static SignalHandler& getInstance();

	/**
	 * @brief Gets the last signal thrown as an integer.
	 *
	 * @return 0 for no signal, SIG* for any of the other signals. The following is a list of some of the most common signals. See "man 7 signal" for a complete list.
	 * <br>
	 * <pre>
	 * 0       No signal
	 * SIGINT  Interrupt from keyboard (Ctrl+C)
	 * SIGABRT abort() called
	 * SIGSEGV Invalid memory access.
	 * SIGTERM Default termination signal.
	 * </pre>
	 * <br>
	 * Note that SIGKILL cannot be caught.
	 */
	sig_atomic_t getLastSignal();

	/**
	 * @brief Gets a string representation of the last signal thrown.
	 */
	const char* getLastSignalString();

	/* Delete the copy constructor and = operator since we do not want the singleton assigned to a variable, since it will be destructed. */
	SignalHandler(const SignalHandler&) = delete;
	void operator=(const SignalHandler&)  = delete;
private:
	/**
	 * @brief Private constructor so the SignalHandler class can't be instantiated.
	 * Use getInstance() to get a reference to the SignalHandler's singleton.
	 */
	SignalHandler();
	jmp_buf s_jmpbuf;
	std::mutex mutex_jmpbuf;
	volatile sig_atomic_t s_signo = 0;
};

/**
 * @brief Define a signal handler like below:<br>
 * ```C++
 * signalHandler{
 *     //your code here
 * }
 * ```
 * <br>
 * When a signal is thrown, execution will begin at the top of this block and continue until the end of the function it is in.
 */
#define signalHandler __signalhandler __sighand; if (setjmp(__signalhandler::s_jmpbuf))

}

#endif
