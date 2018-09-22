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
At the moment, only POSIX systems (Linux, OSX, BSD, etc.) with gcc are supported.

To build the library:

```shell
make
```

This will output a static library called `libsimpletest.a`
This can be passed to `gcc` like any other object file, or it can be linked with `-lsimpletest` if it is copied to an ld library directory (e.g. `/usr/lib`)

To build the library with debug symbols:

```shell
make debug
```

This will also output a static library called `libsimpletest.a`

To build and run the sample test:

```shell
make sample
./sample
```

The sample test's source can be found in [sample.cpp](sample.cpp).

To build and view the documentation (requires [doxygen](www.doxygen.nl) to be installed):

```shell
doxygen
firefox docs/html/globals.html
```

To clean the project folder:

```shell
make clean
```

## Usage
Define a unit test like follows:
```C++
#include "simpletest.hpp"

UNIT_TEST(your_name_here){
	ASSERT(2 + 2 == 4);
}
```

Then, execute all of your unit tests like follows:
```C++
int main(int argc, char** argv){
	return EXECUTE_TESTS();
}
```
The EXECUTE\_TESTS() macro executes all defined tests and returns the number of tests that failed.
Output will be placed on the screen detailing which tests failed and why.

Check the output of the latest line on stdout like follows:
```C++
UNIT_TEST(stdout_check){
	std::cout << "test " << 123 << std::endl;
	EXPECT("test 123");
}
```
The EXPECT() macro fails the test if the latest line on stdout does not match the given string.
Note that EXPECT() only checks the latest line.

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

## License
This project is licensed under the MIT License. See [LICENSE.txt](LICENSE.txt) for details.
