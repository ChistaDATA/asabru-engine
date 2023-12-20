#include <iostream>
#include "tests.h"
#include "TestConnection.h"

int proxy_port;
std::string server_addr;
int server_port;

int main(int argc, char *argv[]) {
    // Run the tests
    runTests();
}
