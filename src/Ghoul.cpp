#include "../include/Ghoul.h"
#include "../include/Manager.h"
#include "../include/ConfigParser.h"

#include <iostream>
#include <pigpio.h>
#include <memory>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>

#define PI 3.14159265

Ghoul::Ghoul(std::string name)
{
	_name = name;
	_horiz_servo = nullptr;
	_vert_servo = nullptr;
	_eyes_gpio_pin = 0;

	 // Default durations for various states (all times in seconds)
	_blink_duration = 0.5;
	_blink_frequency = 7.0;		// TODO: randomize a bit?
	_min_time_to_sleep = 5.0;	// go to sleep after this much time passed without any events received
	_min_time_to_wake = 5.0;	// Stay asleep at least this long once fallen asleep
	gettimeofday(&_time_fell_asleep, 0);
	gettimeofday(&_last_time_awake, 0);
	_state = ASLEEP;

	if (gpioInitialise() < 0)
	{
		std::cout << "\nError initializing gpio for ghoul " << _name << ".\n" << std::flush;
		exit(1);
	}
	else
	{
		std::cout << "\nSuccessfully initialized gpio for ghoul " << _name << ".\n" << std::flush;
	}
}

Ghoul::~Ghoul()
{
	GoToSleep();
}

/********* Setters *********/
void Ghoul::SetHorizServo(ServoConfig *servo_config)
{
	_horiz_servo_config = servo_config;
	std::cout << "\nGhoul::SetHorizServo about to initialize with pin " << _horiz_servo_config->gpio_pin << ".\n" << std::flush;
	_horiz_servo = std::make_shared<pigpioServo>(servo_config->gpio_pin,
												 servo_config->angle_maps,
												 servo_config->offsets);
}

void Ghoul::SetVertServo(ServoConfig *servo_config)
{
	_vert_servo_config = servo_config;
	std::cout << "\nGhoul::SetVertServo about to initialize with pin " << _vert_servo_config->gpio_pin << ".\n" << std::flush;
	_vert_servo = std::make_shared<pigpioServo>(servo_config->gpio_pin,
												 servo_config->angle_maps,
												 servo_config->offsets);
}

void Ghoul::SetEyes(int pin)
{
	_eyes_gpio_pin = pin;
	gpioSetMode(_eyes_gpio_pin, PI_OUTPUT);
}

std::shared_ptr<pigpioServo> Ghoul::GetHorizServo()
{
	return _horiz_servo;
}

std::shared_ptr<pigpioServo> Ghoul::GetVertServo()
{
	return _vert_servo;
}

/********* GETTERS ****************/
std::string Ghoul::GetName()
{
	return _name;
}

ServoConfig* Ghoul::GetHorizServoConfig()
{
	return _horiz_servo_config;
}

ServoConfig* Ghoul::GetVertServoConfig()
{
	return _vert_servo_config;
}

int Ghoul::GetEyes()
{
	return _eyes_gpio_pin;
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
	else if (_eyes_gpio_pin == 0)
	{
		std::cout << "Ghoul: " << _name << " doesn't have gpio pin set for eyes. Not ready." << std::endl;
	}
	else
	{
		std::cout << "Ghoul: " << _name << " is ready to spook!";
		gpioSetMode(_eyes_gpio_pin, PI_OUTPUT);
		GoToSleep();
		return true;
	}

	return false;
}

void Ghoul::OpenEyes()
{
	Manager::ToggleLED(_eyes_gpio_pin, LED_ON);
}

void Ghoul::CloseEyes()
{
	Manager::ToggleLED(_eyes_gpio_pin, LED_OFF);
}

void Ghoul::BlinkEyes()
{
	// TODO: Implement this method?
	CloseEyes();
	time_sleep(0.5);
	OpenEyes();
}

bool Ghoul::ShouldWakeUp(bool motion_detected)
{
	if (_state == AWAKE)
	{
		// already awake
		return false;
	}

	if (!motion_detected)
	{
		// No reason to wake up - no motion detected
		return false;
	}

	timeval now;
	gettimeofday(&now, 0);
	double elapsed = double(now.tv_sec - _time_fell_asleep.tv_sec);

	if (elapsed > _min_time_to_wake)
	{
		// Enough time has elapsed since we fell asleep. Ready to wake up now.
		std::cout << "\nGhoul::ShouldWakeUp --> Elapsed: " << elapsed << ", Comparison:" << _min_time_to_wake << std::endl;
		std::cout << "\nGhoul::ShouldWakeUp --> Returning True!!" << std::endl;
		return true;
	}

	return false;
}

bool Ghoul::ShouldGoToSleep()
{
	if (_state == ASLEEP)
	{
		// already asleep
		return false;
	}

	timeval now;
	gettimeofday(&now, 0);
	double elapsed = double(now.tv_sec- _last_time_awake.tv_sec);

	if (elapsed  > _min_time_to_sleep)
	{
		std::cout << "\nGhoul::ShouldGoToSleep TRUE--> Elapsed: " << elapsed << ", Comparison" << _min_time_to_sleep << std::endl;
		std::cout << "\nGhoul::ShouldGoToSleep --> Returning True!!" << std::endl;
		return true;
	}
	return false;
}

void Ghoul::WakeUp()
{
	OpenEyes();
	gettimeofday(&_last_time_awake, 0);
	_state = AWAKE;
	_vert_servo->TurnToAngle(_vert_servo_config->angle_maps.max_map.angle);
}

void Ghoul::GoToSleep()
{
	// Center the horizontal one
	_horiz_servo->TurnToAngle(_horiz_servo_config->angle_maps.center_map.angle);

	// lower the vertical one
	_vert_servo->TurnToAngle(_vert_servo_config->angle_maps.min_map.angle);
	CloseEyes();
	gettimeofday(&_time_fell_asleep, 0);
	_state = ASLEEP;
}

void Ghoul::ProcessEvent(double distance, double angle, bool motion_found)
{
	// Receive the angle and distance to the closest moving object.
	// First, determine if we should pay any attention to the event. If not, return.
	// If so, pass to tracker.
	if(_state == ASLEEP)
	{
		std::cout << "S";
	}
	else if (_state == AWAKE)
	{
		std::cout << "A";
	}
	else
	{
		std::cout << "U";
	}

	// First check is to see if scanner detected any motion
	if (!motion_found)
	{
		// No motion detected. All we might want to do now is go to sleep
		if (ShouldGoToSleep())
		{
			GoToSleep();
		}
		return;
	}

	if (_state == ASLEEP)
	{
		if (ShouldWakeUp(motion_found))
		{
			WakeUp();
		}
		else
		{
			// Ghoul is asleep and should stay asleep regardless of input
			return;
		}
	}

	// Ghoul is either awake or just woke up. Track away!
	gettimeofday(&_last_time_awake, 0);
	Track(distance, angle);
}

void Ghoul::Track(double distance, double angle)
{
	if (_state == AWAKE)
	{
		// Get the relative angle and distance to the object from the perspective of the servo
		double rel_angle = GetRelativeAngle(angle, distance);

		// not used for anything
		//double rel_dist = GetRelativeDistance(angle, distance); // Don't actually need to call this, but it is available for debugging

		printf("\nGHOUL: %s rel_angle: %4.1f°/", _name.c_str(), rel_angle);

		// Ghoul is 'backwards' wRt sensor, so need to flip this angle by 180
		_horiz_servo->TurnToAngle(180 - rel_angle);
	}
}

/************ HELPERS **************/
double Ghoul::DegreesToRadians(double angle_degrees)
{
	return (angle_degrees * PI) / 180.0;
}

double Ghoul::RadiansToDegrees(double angle_radians)
{
	return (angle_radians * 180.0) / PI;
}

double Ghoul::GetRelativeAngle(double abs_angle_deg, double abs_distance)
{
	/* This method takes in an angle and distance found by the lidar
	   and returns the angle the servo would need to point toward the same spot.

	   For the purposes of this calculation, the lidar is considered to be at the origin of
	   the coordinate plane with the front of the lidar aligned along the positive y axis.
	*/

	// Shortcuts
	double theta_rad = DegreesToRadians(abs_angle_deg);
	double x_off = _horiz_servo->GetOffsets().offsetX; // in mm
	double y_off = _horiz_servo->GetOffsets().offsetY; // in mm
	
	double xLIDAR = abs_distance*cos(theta_rad);
	double yLIDAR = abs_distance*sin(theta_rad);
	
	double inverse_tangent_rad = atan2( (xLIDAR - x_off),(yLIDAR - y_off) );

	double ret_val_rads = PI/2 - inverse_tangent_rad;
	double ret_val_degrees = RadiansToDegrees(ret_val_rads);
	
	if (ret_val_degrees < 0)
	{
		//std::cout << "MINIMAL CORRECTION" << std::endl;
		ret_val_degrees += 360;
	}
	else if (ret_val_degrees > 360)
	{
		//std::cout << "MAXIMAL CORRECTION" << std::endl;
		ret_val_degrees -= 360;
	}
	
	return ret_val_degrees + _horiz_servo->GetOffsets().offsetAngle;
}

double Ghoul::GetRelativeDistance(double abs_angle_deg, double abs_distance)
{
	// This method takes in an angle and distance found by the lidar and returns the absolute distance from the servo to the object.

	// For the purposes of this calculation, the lidar is considered to be at the origin of
	// the coordinate plane with the front of the lidar aligned along the positive y axis.

	// Shortcuts
	double theta_rad = (abs_angle_deg * PI) / 180.0;
	double x_off = _horiz_servo->GetOffsets().offsetX;
	double y_off = _horiz_servo->GetOffsets().offsetY;
	double d = abs_distance;

	// NOTE untested. Didn't bother since I don't actually need it for anything yet. 
	return sqrt( pow(d*cos(theta_rad) - x_off, 2) + pow(d*sin(theta_rad) - y_off, 2) );
}
