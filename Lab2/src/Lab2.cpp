#include <iostream>
#include <string>
using namespace std;


void Println(string message) {
	cout << message << endl;
}

int main() {
	Println("Hello World from QNX Neutrino RTOS!!!");
	Println("@author Amy Novun (novu0001@algonquinlive.com)");
	Println("Unique trait: I hoard dark chocolate in my fridge as an emergency food supply. :)");
	return 0;
}
