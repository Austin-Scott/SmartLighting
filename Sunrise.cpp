#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "lightbulb.h"

using namespace std;

int main() {

	//Connect to smart lighting
	Lightbulb bulb("192.168.1.26");
	Lightbulb bulb1("192.168.1.24");
	Lightbulb bulb2("192.168.1.25");


	float duration = 60 * 20; //Duration in seconds. This simulation will take 20 minutes.

	//Create some color states for the bulbs to use during the simulation.
	RGBColor red(255, 0, 0);
	HSBColor start(red, 0);
	RGBColor warm(255, 228, 206);
	HSBColor end(warm, 50);

	//Schedule each bulbs behavior during the simulation.
	bulb.addHSB(0.0f, 0.75f, start, end);
	bulb.addTemp(0.75f, 1.0f, 50, 100, 2500, 9000);
	bulb.startSchedule(duration);

	bulb1.addOff(0.0f, 0.90f);
	bulb1.addBrightness(0.90f, 1.0f, 0, 100);
	bulb1.startSchedule(duration);

	bulb2.addOff(0.0f, 0.75f);
	bulb2.addBrightness(0.75f, 1.0f, 0, 100);
	bulb2.startSchedule(duration);

	cout << "Commencing sunrise simulation..." << endl;

	auto then = chrono::steady_clock::now();

	while (!bulb.scheduleFinished() || !bulb1.scheduleFinished() || !bulb2.scheduleFinished()) {

		auto now = chrono::steady_clock::now();
		auto delta = now - then;
		then = now;

		bulb.updateBySchedule(chrono::duration_cast<chrono::milliseconds>(delta).count());
		bulb1.updateBySchedule(chrono::duration_cast<chrono::milliseconds>(delta).count());
		bulb2.updateBySchedule(chrono::duration_cast<chrono::milliseconds>(delta).count());

		this_thread::sleep_for(chrono::milliseconds(250));
		
	}

	cout << "Sunrise completed. Have a wonderful day! :)" << endl;

}