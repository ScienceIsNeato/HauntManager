#pragma once

#include "Scanner.h"

#define IS_TRACKING 1
#define NOT_TRACKING 0

class Manager
{
public:
	Manager();
	~Manager();

	void ParseResult(ScanResult scan);

	bool ShouldRecalibrate(int &current_state, int &counter, double angle, double first_angle_detected);
	void SetCalibrationValues(double(calibration_values)[NUM_SAMPLE_POINTS]);
	//double(&calibration_values)[NUM_SAMPLE_POINTS] GetCalibrationValues();


	int manager_state;
	double last_tracking_angle;
	int continuous_tracking_counter;
	double _calibration_values[NUM_SAMPLE_POINTS];
};


