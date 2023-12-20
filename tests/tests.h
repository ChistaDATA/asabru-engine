#ifndef tests_h
#define tests_h

/**
 * @file tests.h This is main test file header - forward declare a function used to execute all the tests here
 */

#ifdef UNITTEST
void runTests();
#else
#	define runTests()
#endif

#endif
