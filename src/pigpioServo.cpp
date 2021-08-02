#include "../include/pigpioServo.h"

// TODO fix this shit
//pigpioServo::pigpioServo(int gpio_pin, AngleMaps boundaries)
//{
//	// create empty offset
//	InitialOffset offset;
//	offset.offsetAngle = 90;
//	offset.offsetX = 0;
//	offset.offsetY = 0;
//
//	// call default constructor
//	pigpioServo servo = pigpioServo(gpio_pin, boundaries, offset);
//}

pigpioServo::pigpioServo(int gpio_pin, AngleMaps boundaries, InitialOffset initial_offset)
{
	SetGpioPin(gpio_pin);
	SetBoundaries(boundaries);
	SetOffset(initial_offset);
	Initialize();
}

pigpioServo::~pigpioServo()
{
	std::cout << "\npigpioServo Destructor called...\n" << std::flush;
	Stop();
}

void pigpioServo::SetBoundaries(AngleMaps boundaries)
{
	_max = boundaries.max_map;
	_center = boundaries.center_map;
	_min = boundaries.min_map;
}

void pigpioServo::SetOffset(InitialOffset offset)
{
	_initial_offset = offset;
}

InitialOffset pigpioServo::GetOffsets()
{
	return _initial_offset;
}

void pigpioServo::SetGpioPin(int pin)
{
	_gpio_pin = pin;
}

bool pigpioServo::Initialize()
{
	_last_pos = _center.pulse_width;
	std::cout << "\nA gpio pin is " << _gpio_pin << " and _last_pos is " << _last_pos << std::flush;
	if (gpioInitialise() < 0)
	{
		std::cout << "\nError initializing gpio.\n" << std::flush;
		return false;
	}

	std::cout << "\ngpioInitialise() was successful\n" << std::flush;

	TurnToAngle(90);
	_last_pos = _center.pulse_width;
	return true;
}

void pigpioServo::Stop()
{
	std::cout << "\nRecentering servo on way out...\n" << std::flush;
	TurnToAngle(_center.angle);
	time_sleep(2.0);
	gpioServo(_gpio_pin, 0);
	gpioTerminate();
}

void pigpioServo::TurnToAngle(double angle)
{
	if(!IsAngleValid(angle))
	{
		std::cout << "\nYou entered and invalid angle, dummy.\n" << std::flush;
		return;
	}

	int new_pos = AngleToPulseWidth(angle);
	int pos = _last_pos;
	int step = 1;

	// check valid range
	if (new_pos > MAX_RANGE || new_pos < MIN_RANGE)
	{
		std::cout << "\nPosition of " << new_pos << " is invalid. Please select a range between " << MIN_RANGE << " and " << MAX_RANGE << ".\n" << std::flush;
		return;
	}

	// sweeping up or down?
	if (new_pos < pos)
	{
		step = -1;
	}

	// Perform the rotation
	while (pos != new_pos)
	{
		//printf("%d \n", pos);
		// TODO: maybe speed things up here?
		gpioServo(_gpio_pin, pos);
		pos += step;
		time_sleep(0.001);
	}
	_last_pos = new_pos;
}

bool pigpioServo::IsAngleValid(double angle)
{
	// TODO: actually do stuff here
	return true;
}

int pigpioServo::AngleToPulseWidth(double angle)
{
	// min angle is largest pulse width (i.e. 0 and 2300)
	// max angle is smallest pulse width (i.e. 180 and 600)
	if (angle > _max.angle)
	{
		std::cout << "WARNING - you just attempted to turn to angle " << angle << " - setting to max angle of " << _max.angle << std::flush;
		return _max.pulse_width;
	}
	else if (angle < _min.angle)
	{
		std::cout << "WARNING - you just attempted to turn to angle " << angle << " - setting to min angle of " << _min.angle << std::flush;
		return _min.pulse_width;
	}
	else if (angle < _center.angle)
	{
		// Turning less than center
		double percent_span = (angle - _min.angle) / (_center.angle - _min.angle);
		double pulse_width_delta = (_center.pulse_width - _min.pulse_width) * percent_span;
		int new_pulse_width = _min.pulse_width + pulse_width_delta;
		return new_pulse_width;
	}
	else if (angle > _center.angle)
	{
		// Turning more than center
		double percent_span = (angle - _center.angle) / (_max.angle - _center.angle);
		double pulse_width_delta = (_max.pulse_width - _center.pulse_width) * percent_span;
		int new_pulse_width = _center.pulse_width + pulse_width_delta;
		return new_pulse_width;
	}
	else
	{
		return _center.pulse_width;
	}
}
