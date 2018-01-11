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

	HSBColor firstMinorChime(0, 100, 100);
	HSBColor secondMinorChime(60, 100, 100);
	HSBColor thirdMinorChime(120, 100, 100);
	HSBColor fourthMinorChime(240, 100, 100);

	HSBColor majorChime(0, 100, 100);

	int chimeDuration = 500;
	int numberOfChimes = 0;
	bool isMajorChime = false;

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
		isMajorChime = true;
		numberOfChimes = hour;
	}
	else if (minute < 22) { //Quarter hour chime
		numberOfChimes = 1;
	}
	else if (minute < 40) { //half hour chime
		numberOfChimes = 2;
	}
	else { //third quarter chime
		numberOfChimes = 3;

	}

	//***TESTING
	numberOfChimes = 4;
	isMajorChime = false;
	
	for (int i = 0; i < numberOfChimes; i++) {
		bulb.turnOff(chimeDuration / 2);
		this_thread::sleep_for(chrono::milliseconds(chimeDuration/2));
		if (isMajorChime) {
			bulb.setHSB(majorChime);
		}
		else {
			if(i==0)
				bulb.setHSB(firstMinorChime, chimeDuration);
			else if(i==1)
				bulb.setHSB(secondMinorChime, chimeDuration);
			else if(i==2)
				bulb.setHSB(thirdMinorChime, chimeDuration);
			else
				bulb.setHSB(fourthMinorChime, chimeDuration);
		}
		this_thread::sleep_for(chrono::milliseconds(chimeDuration));
	}

	bulb.turnOff(chimeDuration / 2);
	this_thread::sleep_for(chrono::milliseconds(chimeDuration/2));

	/*
	auto then = chrono::steady_clock::now();

	while (!bulb.scheduleFinished()) {

		auto now = chrono::steady_clock::now();
		auto delta = now - then;
		then = now;

		//bulb.updateBySchedule(chrono::duration_cast<chrono::milliseconds>(delta).count(), false);

		this_thread::sleep_for(chrono::milliseconds(100));

	}
	*/

	bulb.restoreState(initialState);

}