#include <iostream>
#include <fstream>
#include <regex>

#include "lightbulb.h"

using namespace std;

bool check() { //Returns true is 192.168.1.5 responds to ping
	string result = executeSystemCommand("ping 192.168.1.5 -c 3 -W 1 -q", false);
	regex r(".*, (\\d+) received.*");
	smatch m;
	regex_search(result, m, r);
	if (m.size() == 2) {
		return stoi(m[1])>0;
	}
	else {
		cout << "Regex match error." << endl;
		return false;
	}
}

int main() {
	
	ifstream ifile;
	ifile.open("lastCheck.txt");

	bool lastCheck = false;

	if (ifile) {
		ifile >> lastCheck;
		ifile.close();
	}
	else {
		cout << "Error: couldn't open lastCheck.txt. Exiting." << endl;
		return 0;
	}

	bool currentCheck = check();

	if (lastCheck != currentCheck) {
		if (currentCheck) {
			//turn lights on
			cout << "Checking lights to see if they are off..." << endl;

			Lightbulb bulb1("192.168.1.24");
			Lightbulb bulb2("192.168.1.25");
			Lightbulb lamp("192.168.1.26");

			cout << "Turning on lights..." << endl;

			if (bulb1.getConnected()) {
				bulb1.setAllowTimeouts(false);
				bulb1.turnOn();
			}

			if (bulb2.getConnected()) {
				bulb2.setAllowTimeouts(false);
				bulb2.turnOn();
			}

			if (lamp.getConnected()) {
				lamp.setAllowTimeouts(false);
				lamp.turnOn();
			}

			cout << "...finished." << endl;
		}
		else {
			//turn lights off
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
		ofstream ofile("lastCheck.txt");
		if (ofile) {
			ofile << currentCheck;
		}
		else {
			cout << "Error writing new lastCheck value." << endl;
		}
	}
	else {
		cout << "No change in device status. Exiting." << endl;
	}
	
}