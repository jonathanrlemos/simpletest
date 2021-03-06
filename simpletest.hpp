/** @file simpletest.hpp
 * @brief simpletest include
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __SIMPLETEST_HPP
#define __SIMPLETEST_HPP

#include "simpletest_iocapturer.hpp"
#include "simpletest_signal.hpp"
#include <vector>
#include <stdexcept>

namespace simpletest{

/**
 * @brief Do not throw this exception directly. Use the ASSERT() macro instead.
 * This exception indicates that a test assertion failed.
 */
class FailedAssertion : public std::runtime_error{
public:
	/**
	 * @brief Constructs a FailedAssertion exception.
	 *
	 * @param assertion A string representation of the assertion that failed.
	 */
	FailedAssertion(const char* assertion);
};

/**
 * @brief Asserts that a particular condition is true, failing the test if not.
 *
 * @param assertion The condition to test. Anything that's valid in an "if" statement can be used here.
 *
 * @exception FailedAssertion Thrown if the assertion is false.
 */
#define ASSERT(assertion)\
	/* silences __iocapt unused warning */\
	(void)__iocapt;\
	/* silences __sighand unused warning */\
	(void)__sighand;\
	if (!(assertion)){\
		throw simpletest::FailedAssertion(#assertion);\
	}\
	/* The following line makes sure a semicolon is required. */ \
	(void)0

/**
 * @brief Do not throw this exception directly. Use the EXPECT() macro instead.
 * This exception indicates that an expectation failed.
 */
class FailedExpectation : public std::runtime_error{
public:
	/**
	 * @brief Constructs a FailedExpectation exception.
	 *
	 * @param expected The expected output on stdout.
	 * @param actual   The actual output on stdout.
	 */
	FailedExpectation(const char* expected, const char* actual);
};

/**
 * @brief Expects a particular line on stdout/stderr, failing the test if not.
 *
 * @param expectation A const char* containing the text to expect. This does not include the newline.
 *
 * @exception FailedExpectation Thrown if the last line on stdout/stderr does not match the expectation.
 */
#define EXPECT(expectation)\
	/* silences unused __sighand warning */\
	(void)__sighand;\
	simpletest::__expect(expectation, __iocapt)

/**
 * @brief Do not call this function directly. Use the EXPECT() macro instead.
 *
 * @param str      The string to expect.
 * @param __iocapt The current I/O capturer object.
 */
void __expect(const char* str, simpletest::IOCapturer& __iocapt);

/**
 * @brief Sends a line to stdin.
 *
 * @bug stdin must be empty before SEND() can be used.
 * This means that SEND() can only be called once until stdin is emptied through std::cin or other input fetching functions.
 *
 * @param line The line to send.
 */
#define SEND(line)\
	/* silences unused __sighand warning */\
(void)__sighand;\
__iocapt.sendToStdin(line)

/**
 * @brief Do not call this function directly. Use the UNIT_TEST macro.
 * This function registers a test with the internal test vector.
 *
 * @param test The test to register.
 * @param name The name of the test.
 */
void __registertest(void(*test)(simpletest::IOCapturer&, simpletest::SignalHandler&), const char* name);

/**
 * @brief Do not instantiate this class directly. Use the UNIT_TEST macro.
 * This class is needed to run the __registertest() function at global scope.
 */
class __registerdummy{
public:
	/**
	 * @brief Dummy function used to call __registertest()
	 *
	 * @param test The test to register.
	 * @param name The name of the test.
	 */
	__registerdummy(void(*test)(simpletest::IOCapturer&, simpletest::SignalHandler&), const char* name){
		__registertest(test, name);
	}
};

/**
 * @brief Instantiate a unit test like below:<br>
 * ```C++
 * UNIT_TEST(your_name_here){
 *     TEST_ASSERT(2 + 2 == 4);
 * }
 * ```
 *
 * Tests can be run with the EXECUTE_TESTS() macro.
 */
#define UNIT_TEST(str)\
	/* declare the function so it is visible in the following line */\
	void str(simpletest::IOCapturer& __iocapt, simpletest::SignalHandler& __sighand);\
	/* now register the test by instantiating a __registerdummy */\
	simpletest::__registerdummy __##str(str, #str);\
	/* finally define the function prototype so the unit test can be defined */\
	void str(simpletest::IOCapturer& __iocapt, simpletest::SignalHandler& __sighand)

/**
 * @brief Throws an exception instead of crashing the program when a signal is thrown.
 */
#define HANDLE_SIGNALS()\
	/* silences __iocapt unused warning */\
	(void)__iocapt;\
	ActivateSignalHandler(__sighand)

/**
 * @brief Do not call this function directly. Use the EXECUTE_TESTS macro.
 * This function executes all tests registered through the UNIT_TEST macro.
 *
 * @param argc The command-line argument count.
 * @param argv The command-line arguments.
 */
int __executetests(int argc, char** argv);

/**
 * @brief This function executes all tests registered through the UNIT_TEST macro.
 * This must be called within the main() function, and said function must have the following signature:<br>
 * ```C++
 * int main(int argc, char** argv);
 * ```
 * <br>
 * The parameters must be called "argc" and "argv"
 *
 * @return The number of tests that failed.
 */
#define EXECUTE_TESTS() simpletest::__executetests(argc, argv)

/**
 * @brief By default, all screen output is captured for use with the EXPECT() macro.
 * This macro prints a message directly to the screen.
 *
 * @param __VA_ARGS__ A printf format string and its subsequent varargs, if any.
 *
 * @return The number of characters successfully written.
 */
#define TEST_PRINTF(...)\
	/* silences __iocapt unused warning */\
(void)__iocapt;\
/* activates the signal handler if it wasn't already activated through THROW_INSTANTLY_ON_SIGNAL() */\
ActivateSignalHandler(__sighand);\
__iocapt.printToScreen(__VA_ARGS__)

/**
 * @brief A dummy class used for cleanup purposes.
 * Do not instantiate this class directly, use the SET_CLEANUP() macro instead.
 *
 * Note to self: C++ template classes cannot be split into .hpp and .cpp.
 */
template <class CleanupLambda>
class __cleanupdummy{
public:
	__cleanupdummy(CleanupLambda cl): cl(cl){
	}
	/**
	 * @brief Calls the cleanup lambda set in the constructor.
	 */
	~__cleanupdummy(){
		cl();
	}
private:
	CleanupLambda cl;
};

/**
 * @brief Sets a cleanup lambda that is executed when the test exits.
 *
 * @param lambda A lambda function. For example:<br>
 * ```C++
 * char* cptr = new char[20];
 * int* iptr  = new int[50];
 * SET_CLEANUP([&cptr, &iptr]{
 *     delete[] cptr;
 *     delete[] iptr;
 * });
 * ```
 * <br>
 * This only needs to be used for objects that cannot be cleaned up through RAII.
 */
#define SET_CLEANUP(lambda)\
	/* silences __iocapt unused warning */\
	(void)__iocapt;\
	/* silences __sighand unused warning */\
	(void)__sighand\
	__cleanupdummy __cld(lambda);\
	/* requires semicolon */\
	(void)__cld

}

#endif
