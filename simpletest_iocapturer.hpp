/** @file simpletest_iocapturer.hpp
 * @brief simpletest I/O capturer.
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __CS_CSTEST_IOCAPTURER_HPP
#define __CS_CSTEST_IOCAPTURER_HPP

#include "attribute.hpp"
#include <memory>
#include <string>
#include <cstdarg>

namespace simpletest{

struct IOCapturerImpl;

/**
 * @brief Captures terminal output.
 * There can only be one instance of this class at a time.
 *
 * A singleton is not appropriate here, because this class's destructor is needed to stop the I/O capture.
 */
class IOCapturer{
public:
	/**
	 * @brief Begins capturing std(out/err/in).
	 * This output is not echoed to the screen while it is being captured.
	 * Capture stops when the IOCapturer instance is destructed.
	 *
	 * @exception std::logic_error There is already an instance of this class.
	 * @exception std::runtime_error Failed to create one or more pipes.
	 */
	IOCapturer();

	/**
	 * @brief Stops capturing std(out/err/in).
	 */
	~IOCapturer();

	/**
	 * @brief Returns all of what was sent to stdout/stderr since the last call to getStdout()
	 *
	 * @return stdout/stderr's input, including all newlines.
	 */
	std::string getStdout();

	/**
	 * @brief Returns the last line of getStdout()
	 */
	static std::string getLastLine(std::string input);

	/**
	 * @brief Sends a line to stdin.
	 *
	 * @param line The line you want to send.
	 * If this does not end with a '\n', one will be appended automatically.
	 */
	void sendToStdin(const char* line);

	/**
	 * @brief Prints a line to the screen.
	 * This output of this function is not captured through getStdout() or getStderr()
	 * This function uses printf(3) syntax.
	 *
	 * @param format A printf format string.
	 * @param ... The corresponding printf varargs, if any.
	 *
	 * @return The number of characters successfully written.
	 */
	int AT_PRINTF_LIKE(1) printToScreen(const char* format, ...);

private:
	/**
	 * @brief Stores the number of instances of this class that are active.
	 * If this is greater than 0, trying to instantiate this class will cause an std::logic_error.
	 */
	static int instanceCount;
	std::unique_ptr<IOCapturerImpl> impl;
};

}

#endif
