#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <pigpio.h>
#include <iostream>
#include <memory>
#include "../include/pigpioServo.h"

#include "../include/rplidar/rplidar.h" //RPLIDAR standard sdk, all-in-one header
#include "../include/Scanner.h"
#include "../include/Manager.h"

using namespace rp::standalone::rplidar;

bool ctrl_c_pressed;

// Local helpers
void ctrlc(int)
{
    ctrl_c_pressed = true;
}

RPlidarDriver* CreateDriver()
{
	RPlidarDriver * drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
	if (!drv) {
		fprintf(stderr, "insufficent memory, exit\n");
		exit(-2);
	}
	return drv;
}

void on_finished(RPlidarDriver * drv, Scanner *scanner)
{
	// TODO move this stuff to manager Stop method
	scanner->Stop(drv);
	int tmp;
	std::cin >> tmp;
	scanner->Close(drv);
	exit(0);
}

int main(int argc, char *argv[])
{

	AngleMaps angle_maps;
	int gpio_pin = 17;

	AngleMap center;
	AngleMap right;
	AngleMap left;

	if (argc == 8)
	{
		gpio_pin = atoi(argv[1]);

		right.angle = atoi(argv[2]);
		right.pulse_width = atoi(argv[3]);

		center.angle = atoi(argv[4]);
		center.pulse_width = atoi(argv[5]);

		left.angle = atoi(argv[6]);
		left.pulse_width = atoi(argv[7]);
		std::cout << "\nTODO move loading of servo specs into code instead of cmd line inputs...\n";
	}
	else
	{
		// TODO remove this stuff - config in code
		std::cout << "\nYou called me wrong. Call me like this:\n";
		std::cout << "\tsudo ./servo_tester <gpio_pin> <right_angle> <right_pulse> <center_angle> <center_pulse> <left_angle> <left_pulse>\n\n";
		exit(1);
	}

	angle_maps.center_map = center;
	angle_maps.right_map = right;
	angle_maps.left_map = left;

	InitialOffset offset;
	offset.offsetAngle = 90;
	offset.offsetX = 0;
	offset.offsetY = 0;

	std::shared_ptr<pigpioServo> servo = std::make_shared<pigpioServo>(gpio_pin, angle_maps, offset);

	signal(SIGINT, ctrlc); // set signal handler for control c
	Scanner *scanner = new Scanner();
	RPlidarDriver * drv = CreateDriver(); // create the driver instance - for some reason creating it in the Scanner class crashes the program

	std::shared_ptr<Manager> manager = std::make_shared<Manager>(drv, scanner);

	// TODO Move into manager start method
	if (!(scanner->Start(drv, NULL, NULL))) // Use default com path and baud rate
	{
		on_finished(drv, scanner);
	}
	manager->CalibrateScanner();

	// We want to ignore anything behind the scanner in this demo - servo can't turn that way anyway
	DeadZone dz1 = { 0, 90.0, 0, 10000 };
	DeadZone dz2 = { 270.0, 360.0, 0, 10000 };
	scanner->AddDeadZone(dz1);
	scanner->AddDeadZone(dz2);

	ScanResult res;
	while (!ctrl_c_pressed)
	{
		res = scanner->Scan(drv, manager->calibration_values);
		manager->ParseResult(res);

		if (res.valid && res.closest_distance < manager->calibration_values[res.closest_index])
		{
			printf("\nshortest theta: %03.2f shortest Dist: %08.2f calibration Dist: %08.2f",
				res.closest_angle,
				res.closest_distance,
				manager->calibration_values[res.closest_index]
			);
			double corrected_angle = res.closest_angle - 90;

			if ((corrected_angle > 0.0) && (corrected_angle < 180.0))
			{
				std::cout << "\n sending servo angle of " << corrected_angle << std::flush;
				servo->TurnToAngle(corrected_angle);
			}
			else
			{
				std::cout << "-" << std::flush;
			}
		}
		else
		{
			std::cout << "." << std::flush;
		}
	}

	on_finished(drv, scanner);    
	std::cout << "\nCleaning up...\n";
	return 0;
}


