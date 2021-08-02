#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <pigpio.h>
#include <iostream>
#include <memory>
#include "../include/pigpioServo.h"


int main(int argc, char *argv[])
{
	AngleMaps angle_maps;
	int gpio_pin = 17;

	AngleMap center;
	AngleMap min;
	AngleMap max;

	if (argc == 8)
	{
		gpio_pin = atoi(argv[1]);

		min.angle = atoi(argv[2]);
		min.pulse_width = atoi(argv[3]);

		center.angle = atoi(argv[4]);
		center.pulse_width = atoi(argv[5]);

		max.angle = atoi(argv[6]);
		max.pulse_width = atoi(argv[7]);
		std::cout << "\nGood job!\n";
		std::cout << "center angle is " << center.angle << "and pulse is " << center.pulse_width << std::endl;
	}
	else
	{
		std::cout << "\nYou called me wrong. Call me like this:\n";
		std::cout << "    sudo ./servo_tester <gpio_pin> <min_angle> <min_pulse> <center_angle> <center_pulse> <max_angle> <max_pulse>\n\n";
		exit(1);
	}

	angle_maps.center_map = center;
	angle_maps.min_map = min;
	angle_maps.max_map = max;

	InitialOffset offset;
	offset.offsetAngle = 90;
	offset.offsetX = 0;
	offset.offsetY = 0;

	std::shared_ptr<pigpioServo> servo = std::make_shared<pigpioServo>(gpio_pin, angle_maps, offset);

	for (int i = 0; i < 180; i += 10)
	{
		std::cout << "\nTurning servo to " << i << " degrees...\n" << std::flush;
		servo->TurnToAngle(i);
		time_sleep(2.0);
	}

	for (int i = 180; i > 0; i -= 10)
	{
		std::cout << "\nTurning servo to " << i << " degrees...\n" << std::flush;
		servo->TurnToAngle(i);
		time_sleep(2.0);
	}

	std::cout << "\nCleaning up...\n";
	return 0;
}

