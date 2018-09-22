/** @file simpletest.cpp
 * @brief simpletest main file.
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

// prototypes, std::vector, std::runtime_error
#include "simpletest.hpp"

// std::optional
#include <optional>
// std::for_each
#include <algorithm>
// std::cout, std::cerr
#include <iostream>
// std::setw
#include <iomanip>
// std::strlen
#include <cstring>

namespace simpletest{

/**
 * @brief Computes the number of digits in a number.
 *
 * @param An integer value.
 *
 * @return The number of digits in said number.
 */
template <typename T>
static T AT_CONST nDigits(T x){
	T val = 1;
	T ctr = 0;
	while (val <= x){
		val *= 10;
		ctr++;
	}
	return ctr;
}

/**
 * @brief A class that contains a unit test and its failure reason, if any.
 */
class UnitTest{
public:
	/**
	 * @brief Constructs a UnitTest with an empty failure reason.
	 */
	UnitTest(void(*func)(IOCapturer&, SignalHandler&), const char* name): func(func), name(name){
	}

	/**
	 * @brief Copy constructor for UnitTest
	 */
	UnitTest(const UnitTest& other){
		func = other.func;
		name = other.name;
	}

	/**
	 * @brief Gets the test's function pointer.
	 *
	 * @return A test that takes an IOCapturer reference and a SignalHandler reference as parameters and returns void.
	 */
	void (*getFunc() const)(IOCapturer&, SignalHandler&){
		return func;
	}

	/**
	 * @brief Gets the name of the test.
	 *
	 * @return The function name of the test.
	 */
	const char* getName() const{
		return name;
	}

private:
	void(*func)(IOCapturer&, SignalHandler&);
	const char* name;
};

struct FailedTestInfo{
	FailedTestInfo(size_t index, const char* name, const char* reason): index(index), name(name), reason(std::string(reason)){
	}
	size_t index;
	const char* name;
	std::string reason;
};

/**
 * @brief Prints the results of the testing.
 *
 * @param __testvec The test vector retrieved through __gettestvec()
 * @param __failvec The tests that failed.
 */
static void printResults(size_t __testvec_size, std::vector<FailedTestInfo>& __failvec){
	size_t totalLen;
	// first, determine the maximum length of the unit test names
	// this is so we can put the correct number of '.''s so everything ends up aligned

	// get the length of the first test
	size_t maxLen = std::strlen(__failvec[0].name);
	// for each test from {1..end}
	std::for_each(__failvec.begin() + 1, __failvec.end(), [&maxLen](const auto& elem){
		// if its name's length is greater than our max
		if (std::strlen(elem.name) > maxLen){
			// set our max to the length of that test's name
			maxLen = std::strlen(elem.name);
		}
	});

	// Get the greater number of digits for alignment purposes
	totalLen = nDigits(__testvec_size - __failvec.size() > __failvec.size() ? __testvec_size - __failvec.size() : __failvec.size());
	// output the numbers of tests that passed and failed
	std::cout << std::endl;
	std::cout << "Results:" << std::endl;
	std::cout << std::setw(totalLen) << __testvec_size - __failvec.size() << " Passed" << std::endl;
	std::cout << std::setw(totalLen) << __failvec.size() << " Failed" << std::endl;
	std::cout << std::endl;

	if (__failvec.size() == 0){
		std::cout << "No failed tests" << std::endl;
		return;
	}

	std::cout << "Failed tests:" << std::endl;
	// for each test
	std::for_each(__failvec.begin(), __failvec.end(), [__testvec_size, maxLen](const auto& elem){
		// output "Test [index] ([func_name])"
		// std::setw(nDigits) makes sure that all instances are aligned.
		std::cout << "Test " << std::left << std::setw(nDigits(__testvec_size)) << elem.index + 1 << " (" << elem.name << ')';
		// output 3 dots + the difference between the length of this string and the longest.
		// this makes sure that all lines are aligned
		for (size_t i = 0; i < maxLen - std::strlen(elem.name) + 3; ++i){
			std::cout << '.';
		}
		// output ([reason for failure])
		std::cout << '(' << elem.reason << ')' << std::endl;
	});
}

/**
 * @brief Returns the test vector.
 * This function is needed so the test vector is initialized before any __registertest() functions are called.
 */
static std::vector<UnitTest>& __gettestvec(){
	static std::vector<UnitTest> __testvec;
	return __testvec;
}

/**
 * @brief Executes the tests.
 *
 * @param __testvec The vector of unit tests to execute.
 *
 * @return A vector containing the unit tests that failed, along with their indexes within the test vector.
 */
static std::vector<FailedTestInfo> runTests(std::vector<UnitTest>& __testvec){
	size_t i = 0;
	size_t maxLen = 0;
	std::vector<FailedTestInfo> __failvec;

	if (__testvec.size() == 0){
		return {};
	}

	maxLen = std::strlen(__testvec[0].getName());
	std::for_each(__testvec.begin() + 1, __testvec.end(), [&maxLen](const auto& elem){
		if (std::strlen(elem.getName()) > maxLen){
			maxLen = std::strlen(elem.getName());
		}
	});

	for (; i < __testvec.size(); ++i){
		std::cout << "Test " << std::left << std::setw(nDigits(__testvec.size())) << i + 1 << " (" << __testvec[i].getName() << ")";
		for (size_t j = 0; j < maxLen - std::strlen(__testvec[i].getName()) + 3; ++j){
			std::cout << '.';
		}

		try{
			{
				IOCapturer __iocapt;
				SignalHandler __sighand;
				__testvec[i].getFunc()(__iocapt, __sighand);
			}
			std::cout << "Passed";
		}
		catch (FailedAssertion& e){
			__failvec.push_back(FailedTestInfo(i, __testvec[i].getName(), e.what()));
			std::cout << "Failed: " << e.what();
		}
		catch (FailedExpectation& e){
			__failvec.push_back(FailedTestInfo(i, __testvec[i].getName(), e.what()));
			std::cout << "Failed: " << e.what();
		}
		catch (SignalException& e){
			const std::string failureReason = std::string("Signal thrown: ") + e.what();
			__failvec.push_back(FailedTestInfo(i, __testvec[i].getName(), failureReason.c_str()));
			std::cout << failureReason;
		}
		catch (std::exception& e){
			const std::string failureReason = std::string("Internal error: ") + e.what();
			__failvec.push_back(FailedTestInfo(i, __testvec[i].getName(), failureReason.c_str()));
			std::cout << failureReason;
		}
		catch (...){
			__failvec.push_back(FailedTestInfo(i, __testvec[i].getName(), "Unknown internal error"));
			std::cout << "Unknown internal error";
		}
		std::cout << std::endl;
	}

	return __failvec;
}

FailedAssertion::FailedAssertion(const char* assertion): std::runtime_error(assertion) {
}

FailedExpectation::FailedExpectation(const char* expected, const char* actual): std::runtime_error('\"' + std::string(expected) + "\" == \"" + actual + '\"'){
}

void __expect(const char* str, IOCapturer& __iocapt){
	std::string q = IOCapturer::getLastLine(__iocapt.getStdout());
	if (str != q){
		throw FailedExpectation(str, q.c_str());
	}
}

void __registertest(void(*test)(IOCapturer&, SignalHandler&), const char* name){
	__gettestvec().push_back(UnitTest(test, name));
}

int __executetests(int argc, char** argv){
	std::vector<UnitTest>& __testvec = __gettestvec();
	std::vector<FailedTestInfo> __failvec;

	(void)argc;
	(void)argv;

	__failvec = runTests(__testvec);

	printResults(__testvec.size(), __failvec);

	return __failvec.size();
}

}
