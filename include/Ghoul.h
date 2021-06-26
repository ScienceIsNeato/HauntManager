#ifndef __GHOUL_H__
#define __GHOUL_H__

#include "../include/pigpioServo.h"
#include <memory>

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
		void WakeUp();
		void GoToSleep();
		void Track();
};

#endif // __GHOUL_H__
