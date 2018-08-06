#include "../include/Manager.h"
#include <cmath>
#include <memory>
#include <cstring>


Manager::Manager()
{
	_num_samples_required = RECALIBRATION_DURATION / SCANNING_INTERVAL;
	_recalibration_samples = 0;
	_contemplating_recalibration = false;
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
		// recalibrate
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
		// Add sample to bin
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

	//std::copy(std::begin(calibration_values), std::end(calibration_values), std::begin(_calibration_values));
}

//double calibration_values[] Manager::GetCalibrationValues()
//{
//	return _calibration_values;
//}

bool Manager::ShouldRecalibrate()
{
	// TODO check the bins

	return false;
}


