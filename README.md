# simpletest
A simple testing framework written in C++17.

## Features
* Include the framework with a single header (`#include "simpletest.hpp"`).
* Unit tests that are easy to define and execute.
* A single ASSERT() macro that supports any if-statement valid expression.
* An EXPECT() macro for checking output on stdout.
* A SEND() macro for sending input to stdin.
* Signal handling to report segmentation faults.
* A complete set of Doxygen-based documentation.
* Written in a way that is somewhat easy to understand.

## Getting started

### Requirements
* POSIX-compatible OS (Linux, OSX, BSD, Cygwin, etc.)
* g++ or clang++ with C++17 support

### Building the library
To build the library with `g++`:

```shell
make
```

To build the library with `clang++`:

```shell
make CXX=clang++
```

Append any `make` commands with `CXX=clang++` to use `clang++` instead of `g++`

Either of these commands will output a static library called `libsimpletest.a`

See below for instructions on linking the library.

### Linking the library
The library can be passed to `g++` or `clang++` like any other object file.
```shell
g++ mytest.cpp libsimpletest.a -std=c++17 -o mytest
```

Alternatively, it can be linked like follows if the library is copied to an `ld` include directory (e.g. `/usr/lib`)
```shell
g++ mytest.cpp -lsimpletest -std=c++17 -o mytest
```

### Debug library
To build the library with debug symbols:

```shell
make debug
```

This will also output a static library called `libsimpletest.a` that can be used the same way as the release library.

### Demo test
To build and run the demo test:

```shell
make demo
./demo
```

The sample test's source can be found in [demo.cpp](demo.cpp).

### Documentation
To build and view the documentation (requires [Doxygen](http://www.doxygen.nl)):

```shell
doxygen
firefox docs/html/index.html # use your favorite browser in place of firefox
```

### Cleaning the project folder
To clean the project folder:

```shell
make clean
```

## Usage

### Making and running unit tests

First, make your unit tests like below:
```C++
#include "simpletest.hpp"

UNIT_TEST(my_test){
	ASSERT(2 + 2 == 4);
}

UNIT_TEST(myTestNumber2){
	int i = 7;
	ASSERT(i == 7);
}
```

Then, execute all of your unit tests like follows:
```C++
int main(int argc, char** argv){
	return EXECUTE_TESTS();
}
```

Compiling and running the program will produce the following output:
```shell
[user@computer simpletest]$ ./demo
Test 1 (my_test).........Passed
Test 2 (myTestNumber2)...Failed: i == 6

Results:
1 Passed
1 Failed

Failed tests:
Test 2 (myTestNumber2)...Failed: i == 6
[user@computer simpletest]$
```
The EXECUTE\_TESTS() macro executes all defined tests and returns the number of tests that failed.
Output will be placed on the screen detailing which tests failed and why.

### Testing stdout

Check the output of the latest line on stdout like follows:
```C++
UNIT_TEST(stdout_check){
	std::cout << "test " << 123 << std::endl;
	EXPECT("test 123");
}
```
The EXPECT() macro fails the test if the latest line on stdout does not match the given string.
Note that EXPECT() only checks the latest line.

### Sending to stdin

Send a line to stdin like follows:
```C++
UNIT_TEST(stdin_send){
	int i;
	std::cout << "enter a number:";
	SEND("123");
	std::cin >> i;
	std::cout << i;
	EXPECT("enter a number:123");
}
```
The SEND() macro sends input to stdin as it it was typed through the keyboard.
A newline will automatically be appended if one is not present in the input string. However, this newline is not reflected in stdout.
Note that stdin must be empty before SEND() can be used, so it will fail if SEND() is used twice before stdin is read.

### Handling segmentation faults

Signals can be caught like follows:
```C++
UNIT_TEST(segfault){
	HANDLE_SIGNALS();
	int* ptr = NULL;
	*ptr = 1234; // segmentation fault here
}
```

The HANDLE\_SIGNALS() macro will report when signals such as SIGSEGV occur instead of letting them crash the program.
Note that is this macro must be placed before any signal-throwing code or it will have no effect.

## License
This project is licensed under the MIT License. See [LICENSE.txt](LICENSE.txt) for details.
