#include "../include/Ghoul.h"
#include "../include/Manager.h"
#include <iostream>
#include <pigpio.h>
#include <memory>

Ghoul::Ghoul(std::string name)
{
	 _name = name;
	 _horiz_servo = nullptr;
	 _vert_servo = nullptr;
	 _left_eye_gpio_pin = 0;
	 _right_eye_gpio_pin = 0;
}

Ghoul::~Ghoul()
{
	GoToSleep();
}

/********* Setters *********/
void Ghoul::SetHorizServo(ServoConfig *servo_config)
{
	_horiz_servo_config = servo_config;
	_horiz_servo = std::make_shared<pigpioServo>(servo_config->gpio_pin,
												 servo_config->angle_maps,
												 servo_config->offsets);
}

void Ghoul::SetVertServo(ServoConfig *servo_config)
{
	_vert_servo_config = servo_config;
	_vert_servo = std::make_shared<pigpioServo>(servo_config->gpio_pin,
												 servo_config->angle_maps,
												 servo_config->offsets);
}

void Ghoul::SetLeftEye(int pin)
{
	_left_eye_gpio_pin = pin;
	gpioSetMode(_left_eye_gpio_pin, PI_OUTPUT);
}

void Ghoul::SetRightEye(int pin)
{
	_right_eye_gpio_pin = pin;
	gpioSetMode(_right_eye_gpio_pin, PI_OUTPUT);
}

std::shared_ptr<pigpioServo> Ghoul::GetHorizServo()
{
	return _horiz_servo;
}

std::shared_ptr<pigpioServo> Ghoul::GetVertServo()
{
	return _vert_servo;
}

/********* STATE MACHINE **********/

bool Ghoul::Ready()
{
	if (_horiz_servo == nullptr)
	{
		std::cout << "Ghoul: " << _name << " doesn't have horizontal servo set. Not ready." << std::endl;
	}
	else if (_vert_servo == nullptr)
	{
		std::cout << "Ghoul: " << _name << " doesn't have vertical servo set. Not ready." << std::endl;
	}
	else if (_left_eye_gpio_pin == 0)
	{
		std::cout << "Ghoul: " << _name << " doesn't have left eye pin set. Not ready." << std::endl;
	}
	else if (_right_eye_gpio_pin == 0)
	{
		std::cout << "Ghoul: " << _name << " doesn't have right eye pin set. Not ready." << std::endl;
	}
	else
	{
		std::cout << "Ghoul: " << _name << " is ready to spook!";
		PrintConfig();
		return true;
	}

	return false;
}

void Ghoul::PrintConfig()
{
	std::cout << "\nGHOUL NAME:  " << _name;

	std::cout << "\n  HORIZONTAL SERVO:     " << _horiz_servo_config->name;
	std::cout << "\n    gpio_pin:           " << _horiz_servo_config->gpio_pin;
	std::cout << "\n    right.angle:        " << _horiz_servo_config->angle_maps.right_map.angle;
	std::cout << "\n    center.angle:       " << _horiz_servo_config->angle_maps.center_map.angle;
	std::cout << "\n    left.angle:         " << _horiz_servo_config->angle_maps.left_map.angle;
	std::cout << "\n    right.pulse_width:  " << _horiz_servo_config->angle_maps.right_map.pulse_width;
	std::cout << "\n    center.pulse_width: " << _horiz_servo_config->angle_maps.center_map.pulse_width;
	std::cout << "\n    left.pulse_width:   " << _horiz_servo_config->angle_maps.left_map.pulse_width;
	std::cout << "\n    offsetAngle:        " << _horiz_servo_config->offsets.offsetAngle;
	std::cout << "\n    offsetX:            " << _horiz_servo_config->offsets.offsetX;
	std::cout << "\n    offsetY:            " << _horiz_servo_config->offsets.offsetY << std::endl;

	std::cout << "\n  VERTICAL SERVO:       " << _vert_servo_config->name;
	std::cout << "\n    gpio_pin:           " << _vert_servo_config->gpio_pin;
	std::cout << "\n    right.angle:        " << _vert_servo_config->angle_maps.right_map.angle;
	std::cout << "\n    center.angle:       " << _vert_servo_config->angle_maps.center_map.angle;
	std::cout << "\n    left.angle:         " << _vert_servo_config->angle_maps.left_map.angle;
	std::cout << "\n    right.pulse_width:  " << _vert_servo_config->angle_maps.right_map.pulse_width;
	std::cout << "\n    center.pulse_width: " << _vert_servo_config->angle_maps.center_map.pulse_width;
	std::cout << "\n    left.pulse_width:   " << _vert_servo_config->angle_maps.left_map.pulse_width;
	std::cout << "\n    offsetAngle:        " << _vert_servo_config->offsets.offsetAngle;
	std::cout << "\n    offsetX:            " << _vert_servo_config->offsets.offsetX;
	std::cout << "\n    offsetY:            " << _vert_servo_config->offsets.offsetY << std::endl;

	std::cout << "\n  Left Eye LED Pin:     " << _left_eye_gpio_pin << std::endl;
	std::cout << "\n  Right Eye LED Pin:    " << _right_eye_gpio_pin << std::endl;
}

void Ghoul::OpenEyes()
{
	Manager::ToggleLED(_left_eye_gpio_pin, LED_ON);
	Manager::ToggleLED(_right_eye_gpio_pin, LED_ON);
}

void Ghoul::CloseEyes()
{
	Manager::ToggleLED(_left_eye_gpio_pin, LED_OFF);
	Manager::ToggleLED(_right_eye_gpio_pin, LED_OFF);
}

void Ghoul::BlinkEyes()
{
	CloseEyes();
	time_sleep(0.5);
	OpenEyes();
}

void Ghoul::WakeUp()
{
	OpenEyes();
	// Stand up
	std::cout << "Ghoul::WakeUp not fully implemented./n";
}

void Ghoul::GoToSleep()
{
	std::cout << "Ghoul::GoToSleep not yet implemented./n";
	// Go back down
	CloseEyes();
}

void Ghoul::Track()
{
	std::cout << "Ghoul::Track not yet implemented./n";
}