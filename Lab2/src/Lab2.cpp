/*	Lab 2: Hello World
 *  Author: Amy Novun
 *
 *	Refs:
 *	[1] cplusplus.com, 2023. "<cstdio>: vsnprintf",
 *		- https://cplusplus.com/reference/cstdio/vsnprintf/
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <stdarg.h>
using namespace std;

void Print(const std::string& message) {
	std::cout << message;
}

void Println(const std::string& message) {
    Print(message);
	std::cout << std::endl;
}

void Printf(const char* format, ...) {
    char buffer[1024]; // Temporary buffer for formatted output
    va_list args;
    va_start(args, format); // Start variadic argument processing
    vsnprintf(buffer, sizeof(buffer), format, args); // Format the string
    va_end(args);

    Print(buffer);
}


int main() {
    // Prompt user to press 'q' to quit.
	std::string input;
    while (true) {
    	Println("---\nHello World from QNX Neutrino RTOS!!!");
    	Println("@author Amy Novun (novu0001@algonquinlive.com)\n");

    	Println("Unique trait: "
    			"I hoard dark chocolate in my fridge as an emergency food supply. :)\n");

        Printf("My PID is: %d \nMy Parent's PID is: %d\n\n", getpid(), getppid());

		Println("Enter 'q' to quit:");
        std::getline(std::cin, input); // Reads the entire line, including blank lines

        // If the input is 'q', break the loop
        // (and ignore all other inputs)
        if (input == "q") {
            std::cout << "Goodbye!" << std::endl;
            break; // Exit the loop
        }
    }
	return 0;
}
