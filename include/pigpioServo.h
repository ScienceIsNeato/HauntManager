#ifndef __PIGPIO_SERVO_H__
#define __PIGPIO_SERVO_H__

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <pigpio.h>
#include <iostream>
#include <string>

#define NUM_GPIO 32
#define MIN_PULSE_WIDTH 500
#define MAX_PULSE_WIDTH 2500
#define DEFAULT_CENTER 1500
#define MIN_RANGE 500
#define MAX_RANGE 2500

// Used when initializing the object if there is a distance or angle offset
// between the servo and the sensor point
struct InitialOffset
{
	double offsetX; // x distance between servo and sensor
	double offsetY; // y distance between servo and sensor
	double offsetAngle; // angle offset between servo and sensor
};

// A way to map an angle in degrees to a pulsewidth
struct AngleMap
{
	double angle;
	int pulse_width;
};

struct AngleMaps
{
	AngleMap left_map;
	AngleMap center_map;
	AngleMap right_map;
};

struct ServoConfig
{
	std::string name;
	int gpio_pin;
	AngleMaps angle_maps;
	InitialOffset offsets;
	bool is_horizontal;
};

class pigpioServo
{
public:
	//pigpioServo(int gpio_pin, AngleMaps boundaries);
	pigpioServo(int gpio_pin, AngleMaps boundaries, InitialOffset initial_offset);
	~pigpioServo();
	void SetBoundaries(AngleMaps boundaries);
	void SetOffset(InitialOffset offset);
	InitialOffset GetOffsets();
	void SetGpioPin(int pin);
	bool Initialize();
	void Stop();
	void TurnToAngle(double angle);
	bool IsAngleValid(double angle);

private:
	InitialOffset _initial_offset;
	int _gpio_pin;
	AngleMap _max_left;
	AngleMap _center;
	AngleMap _max_right;
	int default_pulsewidth;
	int current_pulsewidth;

	int AngleToPulseWidth(double angle);
	int _last_pos;
};

#endif // __PIGPIO_SERVO_H__
