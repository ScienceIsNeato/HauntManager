#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <pigpio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <math.h>
#include "../include/pigpioServo.h"

#include "../include/rplidar/rplidar.h" //RPLIDAR standard sdk, all-in-one header
#include "../include/Scanner.h"
#include "../include/Manager.h"
#include "../include/Ghoul.h"

#define PI 3.14159265

using namespace rp::standalone::rplidar;

bool finish_now;

// Local helpers
void ctrlc(int)
{
	finish_now = true;
	printf("\n\nCaptured a SIGTERM or SIGINT - Cleaning Up...");
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
	scanner->Close(drv);
}

// TODO move this parser and reader?
bool ReadConfig(std::vector<Ghoul*> &ghouls)
{
	std::cout << "Reading servo configurations...\n";
	std::ifstream infile("servo_config.conf");

	if(!infile)
	{
		std::cout << "Couldn't open `servo_config.conf`. Did you copy over the template like in the README?\n";
		return false;
	}

	ServoConfig *current_config = new ServoConfig();
	Ghoul *current_ghoul;

	//while (infile >> key >> val)
	std::string line;
	std::string delimiter = ":";
	std::size_t delim_pos;
	std::string key;
	std::string val;
	size_t first;
	size_t last;

	// TODO: move all this config reading jazz to a config reader
	while(getline (infile, line))
	{
		// Skip blank lines
		if(line.length() == 0)
		{
			// Blank line - ignore
			continue;
		}
		else if(line.substr(0,1) == "#")
		{
			// Comment - ignore
			continue;
		}

		// Make sure there's a colon on the line
		delim_pos = line.find(delimiter);
		if(delim_pos == std::string::npos)
		{
			std::cout << "Error - no `:` found on this line of config: " << line << ".\n";
			return false;
		}

		// Make sure delimiter isn't the start or end of the line
		if(delim_pos == 0 || delim_pos == line.length())
		{
			std::cout << "Error - config line can't start or end with a colon.\n";
			return false;
		}

		// Grab key-value pair
		key = line.substr(0, delim_pos);
		val = line.substr(delim_pos + 1);

		// Trim any whitespace
		first = key.find_first_not_of(' ');
		last = key.find_last_not_of(' ');
		key = key.substr(first, (last-first+1));

		first = val.find_first_not_of(' ');
		last = val.find_last_not_of(' ');
		val = val.substr(first, (last-first+1));

		if(key == "GHOUL")
		{
			// TODO handle multiple ghouls
			current_ghoul = new Ghoul(val);
		} 
		else if(key == "SERVO_NAME")
		{
			// Found a config for a new servo. Add last one to vector if not already done.
			if(current_config->is_horizontal)
			{
				current_ghoul->SetHorizServo(current_config);
			}
			else
			{
				current_ghoul->SetVertServo(current_config);
			}
			current_config = new ServoConfig();
			current_config->name = val;
		}
		else if(key == "gpio_pin")
		{
			current_config->gpio_pin = atoi(val.c_str());
		}
		else if(key == "right.angle")
		{
			current_config->angle_maps.right_map.angle = atoi(val.c_str());
		}
		else if(key == "center.angle")
		{
			current_config->angle_maps.center_map.angle = atoi(val.c_str());
		}
		else if(key == "left.angle")
		{
			current_config->angle_maps.left_map.angle = atoi(val.c_str());
		}
		else if(key == "right.pulse_width")
		{
			current_config->angle_maps.right_map.pulse_width = atoi(val.c_str());
		}
		else if(key == "center.pulse_width")
		{
			current_config->angle_maps.center_map.pulse_width = atoi(val.c_str());
		}
		else if(key == "left.pulse_width")
		{
			current_config->angle_maps.left_map.pulse_width = atoi(val.c_str());
		}
		else if(key == "offsetAngle")
		{
			current_config->offsets.offsetAngle = atoi(val.c_str());
		}
		else if(key == "offsetX")
		{
			current_config->offsets.offsetX = atoi(val.c_str());
		}
		else if(key == "offsetY")
		{
			current_config->offsets.offsetY = atoi(val.c_str());
		}
		else if(key == "type")
		{
			if (val == "horizontal")
			{
				current_config->is_horizontal = true;
			}
			else
			{
				current_config->is_horizontal = false;
			}
		}
		else if(key == "left_eye_pin")
		{
			current_ghoul->SetLeftEye(atoi(val.c_str()));
		}
		else if(key == "right_eye_pin")
		{
			current_ghoul->SetRightEye(atoi(val.c_str()));
		}
		else
		{
			std::cout << "Unknown KEY, VALUE pair: " << key << ", " << val;
			return false;
		}
	}

	// There's probably another servo that still needs to be added in. This is ugly and should be fixed.
	if(current_config->is_horizontal)
	{
		current_ghoul->SetHorizServo(current_config);
	}
	else
	{
		current_ghoul->SetVertServo(current_config);
	}

	// TODO handle multiple ghouls
	ghouls.push_back(current_ghoul);

	std::cout << "Parsing complete - loaded " << ghouls.size() << " ghoul configurations.\n";

	return true;
}

double GetRelativeAngle(double abs_angle_deg, double abs_distance, InitialOffset servo_offsets)
{
	// This method takes in an angle and distance found by the lidar as well as the relative position
	// of a servo and returns the angle the servo would need to point toward the same spot.

	// For the purposes of this calculation, the lidar is considered to be at the origin of
	// the coordinate plane with the front of the lidar aligned along the positive y axis.

	// 1. Convert angle to radians for math.h
	double abs_angle_rad = (abs_angle_deg * PI) / 180.0;
	// 2. Figure out the absolute x and y postions of the spot

	double y_relative_to_lidar = sin (abs_angle_rad) * abs_distance;
	double x_relative_to_lidar = cos (abs_angle_rad) * abs_distance;

	// 3. Figure out the relative x and y distances to the servo
	double y_relative_to_servo = y_relative_to_lidar + servo_offsets.offsetY;
	double x_relative_to_servo = x_relative_to_lidar + servo_offsets.offsetX;

	// 4. Get inverse tangent from 3.
	double rel_angle_rad = atan(y_relative_to_servo/x_relative_to_servo);

	// 5. Convert back to degrees
	double rel_angle_deg = rel_angle_rad * (180.0 / PI);

	// 6. Account for the initial offset angle of the servo itself
	rel_angle_deg -= servo_offsets.offsetAngle;

	std::cout << "Y is " << y_relative_to_lidar << ", and X is " << x_relative_to_lidar << std::endl;
	std::cout << "Rel Y is " << y_relative_to_servo << ", and Rel X is " << x_relative_to_servo << std::endl;
	std::cout << "Abs angle is " << abs_angle_deg << ", and Rel angle is " << rel_angle_deg << std::endl;

	return rel_angle_deg;
}

int main(int argc, char *argv[])
{
	// Load servo configuration from file
	std::vector<Ghoul*> ghouls;
	bool read_success = ReadConfig(ghouls);

	if(!read_success)
	{
		std::cout << "\n Failure reading servo config file - exiting." << std::endl;
		exit(1);
	}

	Ghoul *ghoul = ghouls[0];
	
	if (!(ghoul->Ready()))
	{
		std::cout << "\n Config read successfully but Ghouls not ready - exiting." << std::endl;
		exit(1);
	}

	std::shared_ptr<pigpioServo> servo = ghoul->GetHorizServo(); // TODO handle multiple
	// END Servo setup

	signal(SIGINT, ctrlc); // set signal handler for control c

	// Handlers for systemd attemps to stop service
	signal(SIGSTOP, ctrlc);
	signal(SIGTSTP, ctrlc);
	signal(SIGTERM, ctrlc);

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
	while (!finish_now)
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

				// Just a little thing to test class
				corrected_angle > 90 ? ghoul->OpenEyes(): ghoul->CloseEyes();
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
