#include <iostream>

#include "lightbulb.h"

using namespace std;

int main() {
	cout << "Checking lights to see if they are on..." << endl;

	Lightbulb bulb1("192.168.1.24");
	Lightbulb bulb2("192.168.1.25");
	Lightbulb lamp("192.168.1.26");

	cout << "Turning off lights..." << endl;

	if (bulb1.getConnected()) {
		bulb1.setAllowTimeouts(false);
		bulb1.turnOff();
	}

	if (bulb2.getConnected()) {
		bulb2.setAllowTimeouts(false);
		bulb2.turnOff();
	}

	if (lamp.getConnected()) {
		lamp.setAllowTimeouts(false);
		lamp.turnOff();
	}

	cout << "...finished." << endl;
}