#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <time.h>

#include "lightbulb.h"

using namespace std;

#ifdef _WIN32

#pragma warning(disable:4996)

#endif

int main() {
	//TODO: take different approach for this problem....


	//Connect to smart lighting
	Lightbulb bulb("192.168.1.26");
	bulb.setAllowTimeouts(false);

	Lightstate initialState = bulb.getState();

	HSBColor minorChime(120, 100, 100);
	HSBColor majorChime(0, 100, 100);

	float chimeDuration = 1.0f;
	
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[10];
	string temp;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 10, "%I", timeinfo);
	temp = string(buffer);
	int hour = stoi(temp); //01-12

	strftime(buffer, 10, "%M", timeinfo);
	temp = string(buffer);
	int minute = stoi(temp); //00-59

	if (minute < 5) { //Top of the hour chime

	}
	else if (minute < 22) { //Quarter hour chime
		bulb.addOff(0.0f, 0.33f);
		bulb.addHSB(0.33f, 0.66f, minorChime, minorChime);
		bulb.addOff(0.66f, 1.0f);

		bulb.startSchedule(chimeDuration * 3);
	}
	else if (minute < 40) { //half hour chime
		bulb.addOff(0.0f, 1.0f / 5.0f);
		bulb.addHSB(1.0f / 5.0f, 2.0f / 5.0f, minorChime, minorChime);
		bulb.addOff(2.0f / 5.0f, 3.0f / 5.0f);
		bulb.addHSB(3.0f / 5.0f, 4.0f / 5.0f, minorChime, minorChime);
		bulb.addOff(4.0f / 5.0f, 1.0f);

		bulb.startSchedule(chimeDuration * 5);
	}
	else { //third quarter chime
		bulb.addOff(0.0f, 1.0f / 7.0f);
		bulb.addHSB(1.0f / 7.0f, 2.0f / 7.0f, minorChime, minorChime);
		bulb.addOff(2.0f / 7.0f, 3.0f / 7.0f);
		bulb.addHSB(3.0f / 7.0f, 4.0f / 7.0f, minorChime, minorChime);
		bulb.addOff(4.0f / 7.0f, 5.0f / 7.0f);
		bulb.addHSB(5.0f / 7.0f, 6.0f / 7.0f, minorChime, minorChime);
		bulb.addOff(6.0f / 7.0f, 1.0f);

		bulb.startSchedule(chimeDuration * 7);
	}
	


	auto then = chrono::steady_clock::now();

	while (!bulb.scheduleFinished()) {

		auto now = chrono::steady_clock::now();
		auto delta = now - then;
		then = now;

		bulb.updateBySchedule(chrono::duration_cast<chrono::milliseconds>(delta).count(), false);

		this_thread::sleep_for(chrono::milliseconds(100));

	}

	bulb.restoreState(initialState);

}