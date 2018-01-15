#include <iostream>

#include "lightbulb.h"

using namespace std;

int main() {
	cout << "Connecting to lights..." << endl;

	Lightbulb bulb1("192.168.1.24");
	Lightbulb bulb2("192.168.1.25");
	Lightbulb lamp("192.168.1.26");

	cout << "Transitioning to Night Mode..." << endl;

	int duration = 30000;

	if (bulb1.getConnected() && bulb1.getState().getIsOn()) {
		bulb1.setAllowTimeouts(false);
		bulb1.turnOff(duration);
	}

	if (bulb2.getConnected() && bulb2.getState().getIsOn()) {
		bulb2.setAllowTimeouts(false);
		bulb2.turnOff(duration);
	}

	if (lamp.getConnected() && lamp.getState().getIsOn()) {
		lamp.setAllowTimeouts(false);
		if (lamp.getState().getBrightness() > 50) {
			lamp.setHSB(0, 100, 50, duration);
		}
		else {
			lamp.setHSB(0, 100, lamp.getState().getBrightness(), duration);
		}
	}

	cout << "Night mode activated. Good night :)" << endl;
}