#include "tests.h"
#include "test_basic.h"
#include "unittest.h"
#include <iostream>

/**
 * @file tests.c This is main test file used to execute all defined unit tests.
 */

// Initialize unit testing framework - this must be done exactly once in the project
UNITTEST_INIT()

#ifdef UNITTEST
void runTests()
{

    std::cout << "Unit test examples" << std::endl;
    UNITTEST_RUNBLOCK_BEGIN();

    UNITTEST_EXEC(testconnection);
//    UNITTEST_EXEC(clickhouseconnection);

    UNITTEST_RUNBLOCK_END();
}
#endif
