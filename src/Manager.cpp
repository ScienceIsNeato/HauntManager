#include "../include/Manager.h"
#include <cmath>
#include <memory>
#include <cstring>


Manager::Manager()
{
}

Manager::~Manager()
{
}

void Manager::ParseResult(ScanResult scan)
{
	if (scan.valid && scan.closest_distance < _calibration_values[scan.closest_index])
	{
		std::cout << "good in here" << std::flush;
	}
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

bool Manager::ShouldRecalibrate(int &current_state, int &counter, double angle, double first_angle_detected)
{
	if (current_state == NOT_TRACKING)
	{
		return false;
	}

	if (std::abs(angle - first_angle_detected < 10)) // TODO make variable
	{
		counter++;
	}

	if (counter > 60) // TODO make variable
	{
		return true;
	}

	return false;
}

