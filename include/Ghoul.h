#ifndef __GHOUL_H__
#define __GHOUL_H__

#include "../include/pigpioServo.h"
#include <memory>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>

enum {AWAKE = 0, ASLEEP =1};

class Ghoul
{
	/* A Ghoul is a collection of the following:
		- a name
		- a horizontal servo
		- a vertical servo
		- Left eye
		- Right Eye
	*/
	private:
		std::string _name;
		ServoConfig *_horiz_servo_config;
		ServoConfig *_vert_servo_config;
		std::shared_ptr<pigpioServo> _horiz_servo;
		std::shared_ptr<pigpioServo> _vert_servo;
		int _left_eye_gpio_pin;
		int _right_eye_gpio_pin;
		int _state; // awake or asleep
		double _blink_duration;
		double _blink_frequency;
		double _min_time_to_sleep;
		double _min_time_to_wake;

		timeval _last_time_awake;
		timeval _time_fell_asleep;

	public:
		Ghoul(std::string name);
		~Ghoul();
		void SetHorizServo(ServoConfig *servo);
		void SetVertServo(ServoConfig *servo);
		std::shared_ptr<pigpioServo> GetHorizServo();
		std::shared_ptr<pigpioServo> GetVertServo();
		void SetLeftEye(int pin);
		void SetRightEye(int pin);
		bool Ready();

		void PrintConfig();
		void OpenEyes();
		void CloseEyes();
		void BlinkEyes();
		bool ShouldWakeUp(bool motion_detected);
		bool ShouldGoToSleep();
		void WakeUp();
		void GoToSleep();
		void Track(double distance, double angle);
		void ProcessEvent(double distance, double angle, bool motion_found = true);
};

#endif // __GHOUL_H__
