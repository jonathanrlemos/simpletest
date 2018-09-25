/** @file simpletest_signal.hpp
 * @brief simpletest signal handler.
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __SIMPLETEST_SIGNAL_HPP
#define __SIMPLETEST_SIGNAL_HPP

#include <csetjmp>
#include <csignal>
#include <iostream>

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
 *
 * A singleton is not appropriate here, because this class's destructor is needed to stop capturing signals, and the destructor for a singleton is never called.
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
	 * @brief Move constructor for SignalHandler.
	 */
	SignalHandler(SignalHandler&& other);

	/**
	 * @brief Move assignment operator for SignalHandler.
	 */
	SignalHandler& operator=(SignalHandler&& other);

	/**
	 * @brief Deleted copy constructor.
	 * There should only be one of this class, so it should not be copied;
	 */
	SignalHandler(const SignalHandler& other) = delete;

	/**
	 * @brief Deleted copy assignment.
	 * There should only be one of this class, so it should not be copied.
	 */
	SignalHandler& operator=(const SignalHandler& val) = delete;

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
	 * @brief Do not call directly. Use the ActivateSignalHandler() macro instead.
	 * Gets a reference to the jump buffer for use with the ActivateSignalHandler() macro.
	 */
	static jmp_buf& getBuf();

	/**
	 * @brief Do not call this function directly. Use the ActivateSignalHandler() macro instead.
	 * True if the function should exit (a signal was called that should terminate the program.
	 */
	static bool shouldExit();
};

/**
 * @brief Activates the signal handler, throwing an exception if a signal is thrown.
 * This allows RAII cleanup to occur when a signal is thrown.
 */
#define ActivateSignalHandler(handler)\
	/* returns 0 on its first call, returns signo (>0) when being jumped to through longjmp() */\
	if (setjmp(simpletest::SignalHandler::getBuf())){\
		/* SIGABRT, SIGINT, etc. */\
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
