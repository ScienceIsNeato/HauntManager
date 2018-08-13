#include "../include/Manager.h"
#include <cmath>
#include <memory>
#include <cstring>


Manager::Manager(RPlidarDriver * drv, Scanner *scanner)
{
	_lidar_driver = drv;
	_scanner = scanner;
	_num_samples_required = (double)RECALIBRATION_DURATION / SCANNING_INTERVAL;
	_recalibration_samples = 0;
	_contemplating_recalibration = false;
	_recent_scans_tracker[36] = { 0 };
	InitializeLEDs();
}

Manager::~Manager()
{
}

/******** LIDAR CALIBRATION *************/
void Manager::CalibrateScanner()
{
	ToggleLED(CALIBRATION_LED_PIN, LED_ON);
	_scanner->Calibrate(_lidar_driver, CALIBRATION_PNTS, calibration_values);
	ToggleLED(CALIBRATION_LED_PIN, LED_OFF);
}

void Manager::StartContemplatingRecalibration()
{
	_contemplating_recalibration = true;
}

bool Manager::ShouldStartContemplatingRecalibration(bool actively_scanning)
{

	return (actively_scanning && !_contemplating_recalibration);
}

void Manager::StopContemplatingRecalibration()
{
	if (ShouldRecalibrate())
	{
		std::cout << "RECALIBRATING!!!!\n" << std::flush;
		CalibrateScanner();
	}
	else
	{
		std::cout << "Decided not to recalibrate...\n" << std::flush;
	}

	_recalibration_samples = 0;
	_contemplating_recalibration = false;

	// reset this dude for next time
	for (int i = 0; i < 36; i++)
	{
		_recent_scans_tracker[i] = 0;
	}
}

void Manager::ContemplateRecalibration(ScanResult scan)
{
	if (!_contemplating_recalibration)
	{
		return;
	}

	if (DetectingSomething(scan))
	{
		// Add sample to bin - cast to int, divide by 10,
		// so 9 goes in index 0, 19 to index 1, 29 to index 2, etc.
		_recent_scans_tracker[((int)scan.closest_angle / 10)]++;
	}

	if (_recalibration_samples >= _num_samples_required)
	{
		StopContemplatingRecalibration();
	}
	_recalibration_samples++;
}

bool Manager::ShouldRecalibrate()
{
	std::cout << "Calculating recalibration...\n" << std::endl;

	int num_bins_populated = 0;
	int samples_collected = 0;
	for (int i = 0; i < 36; i++)
	{
		if (_recent_scans_tracker[i] > 0)
		{
			num_bins_populated++;
			samples_collected += _recent_scans_tracker[i];
			std::cout << "\nfor bin: " << i << "num samples was " << _recent_scans_tracker[i] << std::endl;
		}
	}

	double percent_positive_scans = 100.0*((double)samples_collected / (double)_num_samples_required);

	std::cout << "\npercent positive scans: " << percent_positive_scans << " , threshold: " << RECALIBRATION_PERCENT_THRESHOLD << std::endl;
	std::cout << "\nNum bins populated: " << num_bins_populated << " , threshold: " << RECALIBRATION_BIN_THRESHOLD << std::endl;

	return ((percent_positive_scans >= RECALIBRATION_PERCENT_THRESHOLD) && (num_bins_populated <= RECALIBRATION_BIN_THRESHOLD));
}

/**************** LIGHTING ***************/
void Manager::InitializeLEDs()
{
	if (gpioInitialise() < 0)
	{
		std::cout << "\nError initializing gpio.\n" << std::flush;
		exit(1);
	}
	_calibration_LED_pin = CALIBRATION_LED_PIN;
	gpioSetMode(_calibration_LED_pin, PI_OUTPUT);
}

void Manager::ToggleLED(int pin, int state)
{
	gpioWrite(pin, state);
}


/**************************STATE MACHINE *****************/
void Manager::ParseResult(ScanResult scan)
{
	bool actively_tracking = DetectingSomething(scan);
	if (actively_tracking)
	{
		if (ShouldStartContemplatingRecalibration(true))
		{
			StartContemplatingRecalibration();
		}
		_manager_state = IS_TRACKING;
	}
	else
	{
		_manager_state = NOT_TRACKING;
	}

	ContemplateRecalibration(scan);
}

bool Manager::DetectingSomething(ScanResult scan)
{
	return (scan.valid && scan.closest_distance < calibration_values[scan.closest_index]);
}


