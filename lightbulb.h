#pragma once
#include <iostream>
#include <string>
#include <queue>

#include <iomanip>
#include <sstream>

#include <math.h>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

using namespace std;
using namespace rapidjson;

#ifdef _WIN32

#define popen _popen
#define pclose _pclose

#endif 


int lerp(int a, int b, float alpha) {
	if (alpha < 0.0f) return a;
	else if (alpha > 1.0f) return b;
	else return (int)(((1.0f - alpha)*(float)a) + (alpha*(float)b));
}

string executeSystemCommand(string cmd, bool timeouts=true) {
	//cout << cmd << endl; //****DEBUGGING
	
#ifdef linux
	if(timeouts)
		cmd = "timeout 1s " + cmd;
#endif


	string data;
	FILE * stream;
	const int max_buffer = 256;
	char buffer[max_buffer];
	cmd.append(" 2>&1");
	stream = popen(cmd.c_str(), "r");
	if (stream) {
		while (!feof(stream))
			if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
		pclose(stream);
	}
	return data;
}

class Lightstate {
private:
	bool _on=false;
	string _mode="";
	int _hue=0;
	int _saturation=0;
	int _color_temp=0;
	int _brightness=0;
public:
	Lightstate() {	}
	Lightstate(bool on, string mode, int hue, int saturation, int color_temp, int brightness) {
		_on = on;
		_mode = mode;
		_hue = hue;
		_saturation = saturation;
		_color_temp = color_temp;
		_brightness = brightness;
	}
	bool getIsOn() {
		return _on;
	}
	string getMode() {
		return _mode;
	}
	int getHue() {
		return _hue;
	}
	int getSaturation() {
		return _saturation;
	}
	int getColorTemp() {
		return _color_temp;
	}
	int getBrightness() {
		return _brightness;
	}
};

struct RGBColor {
	int r;
	int g;
	int b;
	RGBColor() {}
	RGBColor(int _r, int _g, int _b) {
		r = _r;
		g = _g;
		b = _b;
	}
	string getHexString() {
		stringstream stream;
		stream << "#"
			<< std::setfill('0') << setw(2) << hex << r
			<< std::setfill('0') << setw(2) << hex << g
			<< std::setfill('0') << setw(2) << hex << b;
		return stream.str();
	}
};

RGBColor lerpRGB(RGBColor a, RGBColor b, float alpha) {
	RGBColor result(lerp(a.r, b.r, alpha), lerp(a.g, b.g, alpha), lerp(a.b, b.b, alpha));
	return result;
}

struct HSBColor {
	int h;
	int s;
	int b;
	HSBColor() {}
	HSBColor(int _h, int _s, int _b) {
		h = _h;
		s = _s;
		b = _b;
	}
	HSBColor(RGBColor c, int brightness) {
		b = brightness;

		float redPrime = (float)c.r / 255.0f;
		float greenPrime = (float)c.g / 255.0f;
		float bluePrime = (float)c.b / 255.0f;

		float* maxValue = &redPrime;
		int colorMax = 1;

		float* minValue = &greenPrime;
		int colorMin = 2;

		if (greenPrime > redPrime) {
			colorMax = 2;
			maxValue = &greenPrime;

			colorMin = 1;
			minValue = &redPrime;
		}
		if (bluePrime > *maxValue) {
			colorMax = 3;
			maxValue = &bluePrime;
		}
		else if (bluePrime < *minValue) {
			colorMin = 3;
			minValue = &bluePrime;
		}

		float colorDelta = *maxValue - *minValue;

		if (colorDelta == 0.0f) {
			h = 0;
		}
		else if (colorMax == 1) {
			float color = (greenPrime - bluePrime) / colorDelta;
			color = fmodf(color, 6.0f);
			h = (int)(60.0f*color);
		}
		else if (colorMax == 2) {
			float color = (bluePrime - redPrime) / colorDelta;
			color += 2.0f;
			h = (int)(60.0f*color);
		}
		else {
			float color = (redPrime - greenPrime) / colorDelta;
			color += 4.0f;
			h = (int)(60.0f*color);
		}

		h %= 360;

		/*
		if (*maxValue == 0.0f) {
			s = 0;
		}
		else {
			s = (int)(colorDelta / (*maxValue));
		}

		if (s > 100) s = 100;
		else if (s < 0) s = 0;
		*/
		//***TESTING
		s = 100;
	}
};

HSBColor lerpHSB(HSBColor a, HSBColor b, float alpha) {
	HSBColor result(lerp(a.h, b.h, alpha), lerp(a.s, b.s, alpha), lerp(a.b, b.b, alpha));
	return result;
}

enum EventType { nothing, on, off, brightness, color, temp, hsb };

class Event {
public:
	EventType _type;
	float _start;
	float _end;

	int _sBrightness;
	int _eBrightness;

	RGBColor _sColor;
	RGBColor _eColor;

	HSBColor _sHSB;
	HSBColor _eHSB;

	int _sTemp;
	int _eTemp;

};

class Lightbulb {
private:
	string _ip;
	string _model;
	string _alias;
	bool _is_color;
	bool _is_dimmable;
	bool _is_variable_color_temp;

	bool _connected;

	bool allowTimeouts;

	Lightstate _state;

	//***Schedules
	float totalDuration;
	float currentDuration;
	queue<Event> schedule;

	void updateEvent(float eventAlpha, int deltaTimeInMilli) {
		if (schedule.front()._type == EventType::nothing) {
			//Do nothing lol
		}
		else if (schedule.front()._type == EventType::on) {
			if (!_state.getIsOn() || _state.getBrightness() != 100) {
				turnOn();
			}
		}
		else if (schedule.front()._type == EventType::off) {
			if (_state.getIsOn()) {
				turnOff();
			}
		}
		else if (schedule.front()._type == EventType::brightness) {
			turnOn(lerp(schedule.front()._sBrightness, schedule.front()._eBrightness, eventAlpha), deltaTimeInMilli);
		}
		else if (schedule.front()._type == EventType::color) {
			setColor(lerpRGB(schedule.front()._sColor, schedule.front()._eColor, eventAlpha).getHexString(), deltaTimeInMilli);
		}
		else if (schedule.front()._type == EventType::temp) {
			turnOn(lerp(schedule.front()._sBrightness, schedule.front()._eBrightness, eventAlpha), deltaTimeInMilli);
			setTemp(lerp(schedule.front()._sTemp, schedule.front()._eTemp, eventAlpha), deltaTimeInMilli);
		}
		else if (schedule.front()._type == EventType::hsb) {
			setHSB(lerpHSB(schedule.front()._sHSB, schedule.front()._eHSB, eventAlpha), deltaTimeInMilli);
		}
	}

public:
	Lightbulb(string ip) {
		_ip = ip;

		allowTimeouts = true;

		for (int i = 0; i < 3; i++) {
			updateDetails();
			if (_connected) break;
		}
		
	}
	void setAllowTimeouts(bool value) {
		allowTimeouts = value;
	}
	void updateDetails() {
		string bulbDetails = executeSystemCommand("tplight details " + _ip);
		if (bulbDetails != "") {
			_connected = true;

			Document doc;
			doc.Parse(bulbDetails.c_str());
			auto iter = doc.FindMember("model");
			if(iter!=doc.MemberEnd())
				_model = iter->value.GetString();

			iter = doc.FindMember("alias");
			if (iter != doc.MemberEnd())
				_alias = iter->value.GetString();

			iter = doc.FindMember("is_color");
			if (iter != doc.MemberEnd())
				_is_color = (bool)iter->value.GetInt();

			iter = doc.FindMember("is_dimmable");
			if (iter != doc.MemberEnd())
				_is_dimmable = (bool)iter->value.GetInt();

			iter = doc.FindMember("is_variable_color_temp");
			if (iter != doc.MemberEnd())
				_is_variable_color_temp = (bool)iter->value.GetInt();

			iter = doc.FindMember("light_state");
			if (iter != doc.MemberEnd()) {
				auto light_state = iter->value.GetObject();
				updateState(light_state);
			}
		}
		else {
			_connected = false;
			cout << "Error: Smartbulb at ip " + _ip + " took too long to respond." << endl;
		}
	}
	template<class T> void updateState(T light_state) {
		bool on;

		auto iter = light_state.FindMember("on_off");
		if (iter != light_state.MemberEnd())
			on = (bool)iter->value.GetInt();

		string mode;
		int hue;
		int saturation;
		int color_temp;
		int brightness;
		if (on) {
			
			iter = light_state.FindMember("mode");
			if (iter != light_state.MemberEnd())
				mode = iter->value.GetString();

			iter = light_state.FindMember("hue");
			if (iter != light_state.MemberEnd())
				hue = iter->value.GetInt();

			iter = light_state.FindMember("saturation");
			if (iter != light_state.MemberEnd())
				saturation = iter->value.GetInt();

			iter = light_state.FindMember("color_temp");
			if (iter != light_state.MemberEnd())
				color_temp = iter->value.GetInt();

			iter = light_state.FindMember("brightness");
			if (iter != light_state.MemberEnd())
				brightness = iter->value.GetInt();
		}
		else {
			iter = light_state.FindMember("dft_on_state");
			if (iter != light_state.MemberEnd()) {
				light_state = iter->value.GetObject();

				iter = light_state.FindMember("mode");
				if (iter != light_state.MemberEnd())
					mode = iter->value.GetString();

				iter = light_state.FindMember("hue");
				if (iter != light_state.MemberEnd())
					hue = iter->value.GetInt();

				iter = light_state.FindMember("saturation");
				if (iter != light_state.MemberEnd())
					saturation = iter->value.GetInt();

				iter = light_state.FindMember("color_temp");
				if (iter != light_state.MemberEnd())
					color_temp = iter->value.GetInt();

				iter = light_state.FindMember("brightness");
				if (iter != light_state.MemberEnd())
					brightness = iter->value.GetInt();
			}
		}
		_state = Lightstate(on, mode, hue, saturation, color_temp, brightness);
	}

	Lightstate getState() {
		return _state;
	}

	bool getConnected() {
		return _connected;
	}

	string getModel() {
		return _model;
	}

	string getAlias() {
		return _alias;
	}

	void restoreState(Lightstate state) {
		if (state.getColorTemp() == 0) { //the bulb was in color mode
			setHSB(state.getHue(), state.getSaturation(), state.getBrightness());
		}
		else { //the bulb was in white light mode
			turnOn(state.getBrightness());
			setTemp(state.getColorTemp());
		}

		if (!state.getIsOn()) turnOff();
	}

	void turnOn(int brightness = 100, int transition = 0) {
		string command = "tplight on -t " + to_string(transition) + " -b " + to_string(brightness) + " " + _ip;

		do {
			string response = executeSystemCommand(command);
			if (response != "") {
				Document doc;
				doc.Parse(response.c_str());
				updateState(doc.GetObject());
				break;
			}
			else {
				_connected = false;
				cout << "Error: Smartbulb at ip " + _ip + " took too long to respond." << endl;
				if (allowTimeouts) {
					cout << "Retrying command..." << endl;
				}
			}
		} while (allowTimeouts);
	}

	void turnOff(int transition = 0) {
		string command = "tplight off -t " + to_string(transition) + " " + _ip;

		do {
			string response = executeSystemCommand(command);
			if (response != "") {
				Document doc;
				doc.Parse(response.c_str());
				updateState(doc.GetObject());
				break;
			}
			else {
				_connected = false;
				cout << "Error: Smartbulb at ip " + _ip + " took too long to respond." << endl;
				if (allowTimeouts) {
					cout << "Retrying command..." << endl;
				}
			}
		} while (allowTimeouts);
	}

	void setColor(int r, int g, int b, int transition = 0) {
		RGBColor temp(r, g, b);
		setColor(temp.getHexString(), transition);
	}

	void setColor(string hex, int transition = 0) {
		if (!_is_color) {
			cout << "Error: Smartbulb at ip " + _ip + " does not support colors." << endl;
			return;
		}
		string command = "tplight hex -t " + to_string(transition) + " " + _ip + " \"" + hex +"\"";


		do {
			string response = executeSystemCommand(command);
			if (response != "") {
				Document doc;
				doc.Parse(response.c_str());
				updateState(doc.GetObject());
				break;
			}
			else {
				_connected = false;
				cout << "Error: Smartbulb at ip " + _ip + " took too long to respond." << endl;
				if (allowTimeouts) {
					cout << "Retrying command..." << endl;
				}
			}
		} while (allowTimeouts);
	}

	void setHSB(int h, int s, int b, int transition = 0) {
		HSBColor temp(h, s, b);
		setHSB(temp, transition);
	}

	void setHSB(HSBColor c, int transition = 0) {
		if (!_is_color) {
			cout << "Error: Smartbulb at ip " + _ip + " does not support colors." << endl;
			return;
		}
		string command = "tplight hsb -t " + to_string(transition) + " " + _ip + " " + to_string(c.h) + " " + to_string(c.s) + " " + to_string(c.b);

		do {
			string response = executeSystemCommand(command);
			if (response != "") {
				Document doc;
				doc.Parse(response.c_str());
				updateState(doc.GetObject());
				break;
			}
			else {
				_connected = false;
				cout << "Error: Smartbulb at ip " + _ip + " took too long to respond." << endl;
				if (allowTimeouts) {
					cout << "Retrying command..." << endl;
				}
			}
		} while (allowTimeouts);
	}

	void setTemp(int k, int transition = 0) {
		if (!_is_variable_color_temp) {
			cout << "Error: Smartbulb at ip " + _ip + " does not support color temperatures." << endl;
			return;
		}
		string command = "tplight temp " + _ip + " -t " + to_string(transition) + " " + to_string(k);

		do {
			string response = executeSystemCommand(command);
			if (response != "") {
				Document doc;
				doc.Parse(response.c_str());
				updateState(doc.GetObject());
				break;
			}
			else {
				_connected = false;
				cout << "Error: Smartbulb at ip " + _ip + " took too long to respond." << endl;
				if (allowTimeouts) {
					cout << "Retrying command..." << endl;
				}
			}
		} while (allowTimeouts);
	}

	void updateBySchedule(int deltaTimeInMilli, bool allowEventSkip=true) {
		if (schedule.size() == 0) return;

		float deltaTime = (float)deltaTimeInMilli / 1000.0f;
		currentDuration += deltaTime;
		float alpha = (currentDuration+deltaTime) / totalDuration;

		if (alpha < schedule.front()._end) {
			float eventAlpha = (alpha - schedule.front()._start) / (schedule.front()._end - schedule.front()._start);
			updateEvent(eventAlpha, deltaTimeInMilli);
		}
		else {
			Event lastEvent;
			if (allowEventSkip) {
				while (alpha > schedule.front()._end && schedule.size() > 0) {
					lastEvent = schedule.front();
					schedule.pop();
				}
			}
			else {
				lastEvent = schedule.front();
				schedule.pop();
			}

			if (schedule.size() == 0) {
				if (lastEvent._type == EventType::brightness) {
					turnOn(lastEvent._eBrightness);
				}
				else if (lastEvent._type == EventType::color) {
					setColor(lastEvent._eColor.getHexString());
				}
				else if (lastEvent._type == EventType::temp) {
					turnOn(lastEvent._eBrightness);
					setTemp(lastEvent._eTemp);
				}
				else if (lastEvent._type == EventType::hsb) {
					setHSB(lastEvent._eHSB);
				}
				return;
			}

			if (schedule.front()._type == EventType::nothing) {
				if (lastEvent._type == EventType::brightness) {
					turnOn(lastEvent._eBrightness, (int)(((totalDuration*schedule.front()._start)*1000)-(currentDuration*1000)));
				} else if (lastEvent._type == EventType::color) {
					setColor(lastEvent._eColor.getHexString(), (int)(((totalDuration*schedule.front()._start) * 1000) - (currentDuration * 1000)));
				} else if (lastEvent._type == EventType::temp) {
					turnOn(lastEvent._eBrightness, (int)(((totalDuration*schedule.front()._start) * 1000) - (currentDuration * 1000)));
					setTemp(lastEvent._eTemp, (int)(((totalDuration*schedule.front()._start) * 1000) - (currentDuration * 1000)));
				} else if (lastEvent._type == EventType::hsb) {
					setHSB(lastEvent._eHSB, (int)(((totalDuration*schedule.front()._start) * 1000) - (currentDuration * 1000)));
				}
			} else if (schedule.front()._type == EventType::on) {
				turnOn(100, (int)(((totalDuration*schedule.front()._start) * 1000) - (currentDuration * 1000)));
			} else if (schedule.front()._type == EventType::off) {
				turnOff((int)(((totalDuration*schedule.front()._start) * 1000) - (currentDuration * 1000)));
			}
			else {
				float eventAlpha = (alpha - schedule.front()._start) / (schedule.front()._end - schedule.front()._start);
				updateEvent(eventAlpha, deltaTimeInMilli);
			}
		}

	}

	bool scheduleFinished() {
		return schedule.size() == 0;
	}

	void startSchedule(float durationInSeconds) {
		totalDuration = durationInSeconds;
		currentDuration = 0.0f;
	}

	void addDoNothing(float start, float end) {
		Event e;
		e._type = EventType::nothing;
		e._start = start;
		e._end = end;
		schedule.push(e);
	}

	void addOn(float start, float end) {
		Event e;
		e._type = EventType::on;
		e._start = start;
		e._end = end;
		schedule.push(e);
	}

	void addOff(float start, float end) {
		Event e;
		e._type = EventType::off;
		e._start = start;
		e._end = end;
		schedule.push(e);
	}

	void addBrightness(float start, float end, int startBrightness, int endBrightness) {
		Event e;
		e._type = EventType::brightness;
		e._start = start;
		e._end = end;

		e._sBrightness = startBrightness;
		e._eBrightness = endBrightness;

		schedule.push(e);
	}

	void addColor(float start, float end, RGBColor startColor, RGBColor endColor) {
		Event e;
		e._type = EventType::color;
		e._start = start;
		e._end = end;

		//Brightness is automatically set to 100

		e._sColor = startColor;
		e._eColor = endColor;

		schedule.push(e);
	}

	void addHSB(float start, float end, HSBColor startHSB, HSBColor endHSB) {
		Event e;
		e._type = EventType::hsb;
		e._start = start;
		e._end = end;

		e._sHSB = startHSB;
		e._eHSB = endHSB;

		schedule.push(e);
	}

	void addTemp(float start, float end, int startBrightness, int endBrightness, int startTemp, int endTemp) {
		Event e;
		e._type = EventType::temp;
		e._start = start;
		e._end = end;

		e._sBrightness = startBrightness;
		e._eBrightness = endBrightness;

		e._sTemp = startTemp;
		e._eTemp = endTemp;

		schedule.push(e);
	}

};