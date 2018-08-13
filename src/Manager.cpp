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
	StopHeartbeat();
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
	bool ret_val = actively_scanning && !_contemplating_recalibration;

	if (ret_val)
	{
		std::cout << "\nStarting potential recalibration routine with " << RECALIBRATION_DURATION << " second duration... \n" << std::flush;
	}

	return ret_val;	
}

void Manager::StopContemplatingRecalibration()
{
	if (ShouldRecalibrate())
	{
		std::cout << "\nRECALIBRATING!!!!\n" << std::flush;
		CalibrateScanner();
	}
	else
	{
		std::cout << "\nDecided not to recalibrate...\n" << std::flush;
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
	std::cout << "\nCalculating recalibration...\n" << std::endl;

	int num_bins_populated = 0;
	int samples_collected = 0;
	for (int i = 0; i < 36; i++)
	{
		if (_recent_scans_tracker[i] > 0)
		{
			num_bins_populated++;
			samples_collected += _recent_scans_tracker[i];
			std::cout << "\n\tFor bin: " << i << "num samples was " << _recent_scans_tracker[i] << std::endl;
		}
	}

	double percent_positive_scans = 100.0*((double)samples_collected / (double)_num_samples_required);

	std::cout << "\nPercent positive scans: " << percent_positive_scans << " , threshold: " << RECALIBRATION_PERCENT_THRESHOLD << std::endl;
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
	Heartbeat();
}

bool Manager::DetectingSomething(ScanResult scan)
{
	return (scan.valid && scan.closest_distance < calibration_values[scan.closest_index]);
}

// These methods are to support a way for the program to let the OS know that it is still running.
// Haven't figured out the issue yet, but sometimes the program becomes unresponsive. So I've got a cron job
// that monitors the heartbeat file and auto-restarts the program if it becomes unresponsive
void Manager::StartHeartbeat()
{
	std::ofstream file_h;
	file_h.open("/tmp/haunt_manager_heartbeat.log", std::ofstream::trunc);
	_heartbeat_fh = file_h;
}

void Manager::Heartbeat()
{
	_heartbeat_fh << "<3\n";
}

void Manager::StopHeartbeat()
{
	_heartbeat_fh.close();
}


