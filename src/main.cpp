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
#include "../include/ConfigParser.h"

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

int main(int argc, char *argv[])
{
	// Load servo configuration from file
	std::vector<Ghoul*> ghouls;
	bool read_success = ConfigParser::ReadConfig(ghouls);

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
		else
		{
			ConfigParser::PrintConfig(ghoul);
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
