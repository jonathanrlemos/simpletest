/** @file tests/cstest/cstest_iocapturer.cpp
 * @brief CloudSync testing framework I/O capturer.
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "simpletest_iocapturer.hpp"

// std::vsnprintf
#include <cstdio>
// std::strlen
#include <cstring>
// fnctl()
#include <fcntl.h>
// stringstream
#include <sstream>
// vector
#include <vector>
// STDOUT_FILENO, dup, dup2, pipe, etc.
#include <unistd.h>

#define P_READ  (0)
#define P_WRITE (1)

namespace simpletest{

int IOCapturer::instanceCount = 0;

/**
 * @brief The private implementation of the IOCapturer class.
 * This is needed to implement the pImpl idiom.
 */
struct IOCapturerImpl{
	/**
	 * @brief A file descriptor to the actual stdout.
	 */
	int stdoutOld = 0;

	/**
	 * @brief A file descriptor to the actual stderr.
	 */
	int stderrOld = 0;

	/**
	 * @brief A file descriptor to the actual stdin.
	 */
	int stdinOld  = 0;

	/**
	 * @brief Two file descriptors making a pipe.
	 * All output sent to stdout while this class is active will instead be sent to stdoutPipe[P_WRITE]
	 * Output can be read from stdoutPipe[P_READ]
	 */
	int stdoutPipe[2] = {0, 0};

	/**
	 * @brief Two file descriptors making a pipe.
	 * All input normally sent to stdin while this class should be sent to stdinPipe[P_WRITE].
	 * This data will be read as normal from any input capturing function/class, such as std::cin.
	 */
	int stdinPipe [2] = {0, 0};
};

IOCapturer::IOCapturer(): impl(std::make_unique<IOCapturerImpl>()){
	if (instanceCount != 0){
		throw std::logic_error("Only one instance of IOCapturer can be active at a time");
	}
	instanceCount++;

	// Create two pipes that will act as our new stdout/stdin
	if (pipe(impl->stdoutPipe) != 0){
		throw std::runtime_error("Failed to create stdout pipe (" + std::string(std::strerror(errno)) + ")");
	}

	if (pipe(impl->stdinPipe) != 0){
		throw std::runtime_error("Failed to create stdin pipe (" + std::string(std::strerror(errno)) + ")");
	}

	// save old stdout/err/in
	impl->stdoutOld = dup(STDOUT_FILENO);
	impl->stderrOld = dup(STDERR_FILENO);
	impl->stdinOld  = dup(STDIN_FILENO);

	// disable buffering. this causes input to be available for our EXPECT() macro instantly and allows stderr and stdout to be synchronized
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	// set out stdout pipe to the stdout fileno
	// this causes all data written to stdout to go to our pipe instead
	dup2(impl->stdoutPipe[P_WRITE], STDOUT_FILENO);
	// send stderr to our stdout pipe.
	dup2(impl->stdoutPipe[P_WRITE], STDERR_FILENO);
	// set stdin pipe to the stdin fileno
	// this causes all stdin reading functions to read from our pipe instead
	dup2(impl->stdinPipe[P_READ], STDIN_FILENO);
}

std::string IOCapturer::getStdout(){
	char buf[1024];
	std::string s = "";
	ssize_t ss;

	// do not block. fail instantly if the operation would block
	fcntl(impl->stdoutPipe[P_READ], F_SETFL, O_NONBLOCK);
	// while there is data in stdin, read it
	while ((ss = read(impl->stdoutPipe[P_READ], buf, sizeof(buf) - 1)) > 0){
		// null terminate the string
		buf[ss] = '\0';
		// add the buffer to our output
		s += buf;
	}
	return s;
}

/*
std::string IOCapturer::getStderr(){
	char buf[1024];
	std::string s = "";
	ssize_t ss;

	fcntl(impl->stderrPipe[P_READ], F_SETFL, O_NONBLOCK);
	while ((ss = read(impl->stderrPipe[P_READ], buf, sizeof(buf) - 1)) > 0){
		buf[ss] = '\0';
		s += buf;
	}
	return s;
}
*/

std::string IOCapturer::getLastLine(std::string input){
	// Return everything after the last '\n' before the string's end.

	// find the last '\n'
	size_t pos = input.find_last_of('\n');
	// if there are none
	if (pos == std::string::npos){
		// this is the last line
		return input;
	}
	// if the string ends with a '\n'
	if (pos == input.size() - 1){
		// remove it
		input.resize(input.size() - 1);
		// do the whole thing again
		return getLastLine(input);
	}
	// otherwise, return everything after the last '\n'
	return std::string(input.c_str() + pos + 1);
}

void IOCapturer::sendToStdin(const char* line){
	char* val = nullptr;
	const char* write_ptr = line;
	// if line does not end with a '\n'
	if (line[std::strlen(line) - 1] != '\n'){
		// val = line + '\n'
		val = new char[std::strlen(line) + 2];
		std::strcpy(val, line);
		std::strcat(val, "\n");
		write_ptr = val;
	}

	// writes to stdin have to be performed in one shot.
	// any subsequent writes to stdin without a corresponding read will fail.
	// this is why we can't just write the string and then the '\n'

	// do not block. if this operation would block, fail instantly.
	fcntl(impl->stdinPipe[P_WRITE], F_SETFL, O_NONBLOCK);

	// if the amount of bytes we wrote does not equal the length of the string
	if (write(impl->stdinPipe[P_WRITE], write_ptr, std::strlen(write_ptr)) != (ssize_t)std::strlen(write_ptr)){
		throw std::runtime_error(std::string("Failed to write to stdin (") + std::strerror(errno) + ")");
	}

	// delete[] nullptr is safe
	delete[] val;
}

int IOCapturer::printToScreen(const char* format, ...){
	std::vector<char> buf(16);
	va_list ap;
	int len;

	// load our varargs
	va_start(ap, format);

	// determine the length of the string
	len = std::vsnprintf(&(buf[0]), 0, format, ap);
	if (len < 0){
		va_end(ap);
		throw std::logic_error("Invalid printf expression");
	}

	// allocate enough memory to hold the string
	buf.resize(len + 1);

	// reload our varargs, since they were used in the last std::vsnprintf() command
	va_end(ap);
	va_start(ap, format);

	// write the string to our buffer
	if (std::vsnprintf(&(buf[0]), len + 1, format, ap) != len){
		va_end(ap);
		throw std::runtime_error("vsnprintf() write error");
	}

	// finally, write the buffer to stdout
	if (write(impl->stdoutOld, &(buf[0]), len) != len){
		va_end(ap);
		throw std::runtime_error("write() error");
	}

	return len;
}

IOCapturer::~IOCapturer(){
	instanceCount--;
	// restore old file descriptors to their respective original numbers
	dup2(impl->stdoutOld, STDOUT_FILENO);
	dup2(impl->stderrOld, STDERR_FILENO);
	dup2(impl->stdinOld,  STDIN_FILENO);
}

}
