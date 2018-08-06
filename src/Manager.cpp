#include "../include/Manager.h"
#include <cmath>
#include <memory>
#include <cstring>


Manager::Manager()
{
	_num_samples_required = (double)RECALIBRATION_DURATION / SCANNING_INTERVAL;
	_recalibration_samples = 0;
	_contemplating_recalibration = false;
	_recent_scans_tracker[36] = { 0 };
}

Manager::~Manager()
{
}

void Manager::StartContemplatingRecalibration()
{
	_contemplating_recalibration = true;
}

bool Manager::ShouldStartContemplatingRecalibration()
{
	return ((_manager_state == NOT_TRACKING) && !_contemplating_recalibration);
}

void Manager::StopContemplatingRecalibration()
{
	if (ShouldRecalibrate())
	{
		std::cout << "RECALIBRATE THIS MOTHERFUCKER!!!!\n";
	}

	_recalibration_samples = 0;
	_contemplating_recalibration = false;
}

void Manager::ContemplateRecalibration(ScanResult scan)
{
	if (!_contemplating_recalibration)
	{
		return;
	}

	if (scan.valid && scan.closest_distance < _calibration_values[scan.closest_index])
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

void Manager::ParseResult(ScanResult scan)
{
	if (scan.valid && scan.closest_distance < _calibration_values[scan.closest_index])
	{
		if (ShouldStartContemplatingRecalibration())
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

void Manager::SetCalibrationValues(double(calibration_values)[NUM_SAMPLE_POINTS])
{
	memcpy(_calibration_values, calibration_values, NUM_SAMPLE_POINTS * sizeof(double)); // int is a POD
}

bool Manager::ShouldRecalibrate()
{
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

	std::cout << "\npercent positive scans: " << percent_positive_scans << " num bins populated: " << num_bins_populated << std::endl;
	
	return ((percent_positive_scans >= RECALIBRATION_PERCENT_THRESHOLD) && (num_bins_populated <= RECALIBRATION_BIN_THRESHOLD));
}



