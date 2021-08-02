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
#include "../include/Ghoul.h"

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
	// TODO: I tried to load an old config and this segfaulted. You should fix that.
	// TODO: Move config reader and printer to its own class

	std::cout << "Reading servo configurations...\n";
	std::ifstream infile("servo_config.conf");

	if(!infile)
	{
		std::cout << "Couldn't open `servo_config.conf`. Did you copy over the template like in the README?\n";
		return false;
	}

	ServoConfig *current_config = nullptr;
	Ghoul *current_ghoul = nullptr;

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
			current_ghoul = new Ghoul(val);
		}
		else if(key == "GHOUL_END")
		{
			ghouls.push_back(current_ghoul);
			current_config = nullptr;
			current_ghoul = nullptr;
		}
		else if(key == "SERVO_NAME")
		{
			current_config = new ServoConfig();
			current_config->name = val;
		}
		else if(key == "SERVO_END")
		{
			if(current_config->is_horizontal)
			{
				current_ghoul->SetHorizServo(current_config);
			}
			else
			{
				current_ghoul->SetVertServo(current_config);
			}
			current_config = nullptr;
		}
		else if(key == "gpio_pin")
		{
			current_config->gpio_pin = atoi(val.c_str());
		}
		else if(key == "min.angle")
		{
			current_config->angle_maps.min_map.angle = atoi(val.c_str());
		}
		else if(key == "center.angle")
		{
			current_config->angle_maps.center_map.angle = atoi(val.c_str());
		}
		else if(key == "max.angle")
		{
			current_config->angle_maps.max_map.angle = atoi(val.c_str());
		}
		else if(key == "min.pulse_width")
		{
			current_config->angle_maps.min_map.pulse_width = atoi(val.c_str());
		}
		else if(key == "center.pulse_width")
		{
			current_config->angle_maps.center_map.pulse_width = atoi(val.c_str());
		}
		else if(key == "max.pulse_width")
		{
			current_config->angle_maps.max_map.pulse_width = atoi(val.c_str());
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

	std::cout << "Parsing complete - loaded " << ghouls.size() << " ghoul configurations.\n";

	return true;
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

	for (auto & ghoul : ghouls)
	{
		if (!(ghoul->Ready()))
		{
			std::cout << "\n Config read successfully but Ghouls not ready - exiting." << std::endl;
			exit(1);
		}
	}

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
			double corrected_angle = scanner->CorrectAngle(res.closest_angle);
			printf("\nLIDAR: dist: %4.1fmm,  angle: %4.1f°/", res.closest_distance, corrected_angle);

			if ((corrected_angle > 0.0) && (corrected_angle < 180.0))
			{
				std::cout << "+" << std::flush;
				for (auto & ghoul : ghouls)
				{
					ghoul->ProcessEvent(res.closest_distance, corrected_angle, true);
				}
			}
			else
			{
				for (auto & ghoul : ghouls)
				{
					ghoul->ProcessEvent(0, 0, false);  // alert ghoul that movement is behind scanner
					std::cout << "-" << std::flush;
				}
			}
		}
		else
		{
			for (auto & ghoul : ghouls)
			{
				ghoul->ProcessEvent(0, 0, false);  // alert ghoul that there's nothing to track
				std::cout << "." << std::flush;
			}
		}
	}

	on_finished(drv, scanner);
	std::cout << "\nCleaning up...\n";
	return 0;
}
