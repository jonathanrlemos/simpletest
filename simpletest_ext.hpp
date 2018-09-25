/** @file simpletest_ext.hpp
 * @brief simpletest extended functions
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __SIMPLETEST_EXT_HPP
#define __SIMPLETEST_EXT_HPP

#include "attribute.hpp"

#include <cstddef>
#include <memory>
#include <vector>
#include <string>

namespace simpletest{

/**
 * @brief Creates a file with the specified data.
 *
 * @param path The path of the file to create.
 * If a file already exists at this path, it will be overwritten.
 * @param mem The memory to fill the file with.
 * @param memLen The length of the aforementioned memory.
 *
 * @exception std::runtime_error There was an error creating/writing to the file. See e.what() for details.
 */
void createFile(const char* path, void* mem, size_t memLen);

/**
 * @brief Compares the contents of two files in memcmp() fashion.
 *
 * @param file1 The path of the first file.
 * @param file2 The path of the second file.
 *
 * @return 0 if both files match, negative if file1 is shorter than file2 or the first character in file1 that does not match is less than file2's, positive otherwise.
 *
 * @exception std::runtime_error Error opening/reading from the file.
 */
int cmpFile(const char* file1, const char* file2);

/**
 * @brief Compares the contents of a file with a block of memory in memcmp() fashion.
 *
 * @param file The file to compare.
 * @param mem The memory to compare the file to.
 * @param memLen The length of the aforementioned memory.
 *
 * @return 0 if the file's contents match the memory's, negative if the file is shorter than the block of memory or the first character in the file that does not match is less than the memory's, positive otherwise.
 *
 * @exception std::runtime_error Error opening/reading from the file.
 */
int cmpFile(const char* file, void* mem, size_t memLen);

/**
 * @brief Compares the contents of a block of memory with a file in memcmp() fashion.
 *
 * @param mem The memory to compare the file to.
 * @param memLen The length of the aformentioned memory.
 * @param file The file to compare.
 *
 * @return 0 if the file's contents match the memory's, negative if the memory is shorter than the file or the first character in the memory that does not match is less than the file's, positive otherwise.
 *
 * @exception std::runtime_error Error opening/reading from the file.
 */
int cmpFile(void* mem, size_t memLen, const char* file);

/**
 * @brief Returns true if a file exists at the path, false otherwise.
 * This will return false if the file is a directory.
 *
 * @param file Path to the file to check the existence of.
 *
 * @return true if a file exists at that path, false if not.
 *
 * @exception std::runtime_error Failed to stat the file in question.
 */
bool fileExists(const char* file);

/**
 * @brief A test environment class.
 * This makes sure that the test environment is cleaned up even when an exception is thrown.
 */
class TestEnvironment{
public:
	/**
	 * @brief Move constructor for TestEnvironment.
	 */
	TestEnvironment(TestEnvironment&& other);

	/**
	 * @brief Move assignment operator for TestEnvironment.
	 */
	TestEnvironment& operator=(TestEnvironment&& other);

	/**
	 * @brief Deleted copy constructor.
	 * This class should not be copied because the destructor deletes files and should only fire once.
	 */
	TestEnvironment(const TestEnvironment& other) = delete;

	/**
	 * @brief Deleted assignment operator.
	 * This class should not be copied because the destructor deletes files and should only fire once.
	 */
	TestEnvironment& operator=(const TestEnvironment& t) = delete;

	/**
	 * @brief Cleans up the test environment.
	 */
	~TestEnvironment();

	/**
	 * @brief Sets up a test environment with the following structure:<br>
	 * <pre>
	 * basePath (0755)
	 *     basePath/file{01..20}.txt (0666)
	 * </pre>
	 * </br>
	 * Each of these files will be initialized with 0 to 4 kb of random characters from A-Z.
	 * The environment will automatically be cleaned up when the function exits.
	 *
	 * @param basePath The path to make the test environment under.
	 * This can be a relative path or an absolute path.
	 *
	 * @return A new TestEnvironment object with the aforementioned structure.
	 *
	 * @exception std::runtime_error Failed to create the directory or its subfiles. See e.what() for details.
	 */
	friend TestEnvironment setupBasicEnvironment(const char* basePath);

	/**
	 * @brief Sets up a test environment with the following structure:<br>
	 * <pre>
	 * basePath (0755)
	 *    basePath/dir1  (0755)
	 *        basePath/dir1/d1file{01..20}.txt (0666)
	 *    basePath/dir2  (0755)
	 *        basePath/dir2/d2file{01..20}.txt (0666)
	 *    basePath/excl  (0755)
	 *        basePath/excl/exfile{01..20}.txt (0666)
	 *        basePath/excl/exfile_noacc.txt   (0000)
	 *    basePath/noacc (0000)
	 */
	friend TestEnvironment setupFullEnvironment(const char* basePath);

	/**
	 * @brief Returns the list of files in the TestEnvironment.
	 */
	std::vector<std::string> getFiles();

private:
	struct TestEnvironmentImpl;
	/**
	 * @brief pImpl idiom.
	 * This unique_ptr contains the private variables of the TestEnvironment class.
	 * This is used so we can #include fewer dependencies and speed up compilation times.
	 */
	 std::unique_ptr<TestEnvironmentImpl> impl;

	/**
	 * @brief Creates a directory within the test environment and fills it with test files.
	 * These files will be created with the following format:
	 * `path/{filePrefix}_{number}`
	 * The number is 1-indexed.
	 *
	 * @param path The directory to create.
	 * If this directory's parent does not exist, this function will fail.
	 *
	 * @param nFiles The number of files to create.
	 * @param filePrefix The prefix to append to each file.
	 */
	TestEnvironment& createTestDirectory(const char* path, const char* filePrefix, int nFiles = 20, size_t maxLen = 4096);

	/**
	 * @brief Private constructor to prevent direct instantiation of TestEnvironment objects.
	 */
	TestEnvironment();
};

/**
 * @brief Fills a block of memory using random letters from 'A' to 'Z'.
 *
 * @param mem The memory to fill.
 * @param len The length of the memory to fill.
 */
void fillMemory(void* mem, size_t len);

namespace rand{
/**
 * @brief Seeds the internal random number generator.
 * A consistent random number generator is needed to generate the same numbers across all platforms.
 *
 * @param seed The seed to use.
 * All subsequent numbers generated will be based on this seed.
 */
void seed(unsigned seed);

/**
 * @brief Returns the next random number in the sequence.
 */
int next();
}

}

#endif
