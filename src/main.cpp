#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <pigpio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "../include/pigpioServo.h"

#include "../include/rplidar/rplidar.h" //RPLIDAR standard sdk, all-in-one header
#include "../include/Scanner.h"
#include "../include/Manager.h"

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
bool ReadServoConfig(std::vector<ServoConfig*> &configs)
{
	std::cout << "Reading servo configurations...\n";
	std::ifstream infile("servo_config.conf");

	if(!infile)
	{
		std::cout << "Couldn't open `servo_config.conf`. Did you copy over the template like in the README?\n";
		return false;
	}

	ServoConfig *current_config = new ServoConfig();

	//while (infile >> key >> val)
	std::string line;
	std::string delimiter = ":";
	std::size_t delim_pos;
	std::string key;
	std::string val;
	size_t first;
	size_t last;

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

		if(key == "SERVO_NAME")
		{
			// Found a config for a new servo. Add last one to vector if not already done.
			if(current_config->name != "")
			{
				configs.push_back(current_config);
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
		else
		{
			std::cout << "Unknown KEY, VALUE pair: " << key << ", " << val;
			return false;
		}
	}

	// Done parsing the config. Add it to the vector and return.
	configs.push_back(current_config);

	std::cout << "Parsing complete - loaded " << configs.size() << " servo configurations.\n";

	return true;
}

void PrintServoConfigs(const std::vector<ServoConfig*> configs)
{
	int count = configs.size();
	if(count < 1)
	{
		std::cout << "No servo configs to print - this is gonna be a problem...\n";
		return;
	}
	std::cout << "Printing " << count << " servo configurations:\n";

	for (int i = 0; i < count; i++)
	{
		std::cout << "\nSERVO_NAME:             " << configs[i]->name;
		std::cout << "\n    gpio_pin:           " << configs[i]->gpio_pin;
		std::cout << "\n    right.angle:        " << configs[i]->angle_maps.right_map.angle;
		std::cout << "\n    center.angle:       " << configs[i]->angle_maps.center_map.angle;
		std::cout << "\n    left.angle:         " << configs[i]->angle_maps.left_map.angle;
		std::cout << "\n    right.pulse_width:  " << configs[i]->angle_maps.right_map.pulse_width;
		std::cout << "\n    center.pulse_width: " << configs[i]->angle_maps.center_map.pulse_width;
		std::cout << "\n    left.pulse_width:   " << configs[i]->angle_maps.left_map.pulse_width;
		std::cout << "\n    offsetAngle:        " << configs[i]->offsets.offsetAngle;
		std::cout << "\n    offsetX:            " << configs[i]->offsets.offsetX;
		std::cout << "\n    offsetY:            " << configs[i]->offsets.offsetY << std::endl;
	}
}

int main(int argc, char *argv[])
{
	// Load servo configuration from file
	std::vector<ServoConfig*> configs;
	bool read_success = ReadServoConfig(configs);
	PrintServoConfigs(configs);

	if(!read_success)
	{
		std::cout << "\n Failure reading servo config file - exiting." << std::endl;
		exit(1);
	}

	ServoConfig *config = configs[0]; // TODO handle multiple
	std::shared_ptr<pigpioServo> servo = std::make_shared<pigpioServo>(config->gpio_pin, config->angle_maps, config->offsets);
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
