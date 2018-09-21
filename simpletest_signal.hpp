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
 * @brief An exception that indicates a signal was thrown.
 */
class SignalException : public std::runtime_error{
public:
	/**
	 * @brief Constructs a SignalException.
	 *
	 * @param signo The signal code.
	 */
	SignalException(sig_atomic_t signo);

	/**
	 * @brief Gets the signal code of this exception.
	 */
	sig_atomic_t getSignal();

private:
	/**
	 * @brief The last signal code.
	 */
	sig_atomic_t signo;
};

/**
 * @brief This class handles signals while it is active.
 * Only one SignalHandler can be active in a thread at once.
 */
class SignalHandler{
public:
	/**
	 * @brief Instantiates a signal handler.
	 * Upon creation, this class will begin capturing SIGINT, SIGABRT, SIGSEGV, and SIGTERM.
	 *
	 * @exception std::logic_error A signal handler has already been instantiated in this thread.
	 */
	SignalHandler();

	/**
	 * @brief Stops capturing signals.
	 */
	~SignalHandler();

	/**
	 * @brief Gets the last signal thrown as an integer.
	 *
	 * @return 0 for no signal, SIG* for any of the other signals. The following is a list of signals caught. Any signal not on this list will have its default behavior.
	 * <br>
	 * <pre>
	 * 0       No signal
	 * SIGINT  Interrupt from keyboard (Ctrl+C)
	 * SIGABRT abort() called
	 * SIGSEGV Invalid memory access.
	 * SIGTERM Default termination signal.
	 * </pre>
	 * <br>
	 */
	static sig_atomic_t lastSignal();

	/**
	 * @brief Gets a string representation of a signal.
	 */
	static const char* signalToString(sig_atomic_t signo);

	/**
	 * @brief Do not call directly. Use the SignalHandlerSetJmp() macro instead.
	 * Gets a reference to the jump buffer for use with the SignalHandlerSetJmp() macro.
	 */
	static jmp_buf& getBuf();

	/**
	 * @brief Do not call this function directly. Use the SignalHandlerSetJmp() macro instead.
	 * True if the function should exit (a signal was called that should terminate the program.
	 */
	static bool shouldExit();
};

/**
 * @brief Activates the signal handler, throwing an exception if a signal is thrown.
 * This allows RAII cleanup to occur when a signal is thrown.
 */
#define ActivateSignalHandler(handler)\
	if (setjmp(SignalHandler::getBuf())){\
		if (handler.shouldExit()){\
			std::cerr << "Terminating program (" << simpletest::SignalHandler::signalToString(handler.lastSignal()) << ")" << std::endl;\
			std::exit(1);\
		}\
		throw simpletest::SignalException(simpletest::SignalHandler::lastSignal());\
	}\
	/* requires the statement to have a semicolon at the end. */\
	(void)0


}

#endif
