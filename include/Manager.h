#pragma once

#include "Scanner.h"

#define IS_TRACKING 1
#define NOT_TRACKING 0
#define RECALIBRATION_DURATION 20 // seconds
#define SCANNING_INTERVAL 0.167 // scan rate is 6 hz

#define RECALIBRATION_PERCENT_THRESHOLD 90.0 // percent of samples that were valid during comtemplation period
#define RECALIBRATION_BIN_THRESHOLD 3 // number of bins populated during contemplation threshold

class Manager
{
public:
	Manager();
	~Manager();

	void ParseResult(ScanResult scan);

	bool ShouldRecalibrate();
	bool ShouldStartContemplatingRecalibration();

	void SetCalibrationValues(double(calibration_values)[NUM_SAMPLE_POINTS]);
	void StartContemplatingRecalibration();
	void StopContemplatingRecalibration();
	void ContemplateRecalibration(ScanResult scan);


	//double(&calibration_values)[NUM_SAMPLE_POINTS] GetCalibrationValues();


	int _manager_state;
	double _calibration_values[NUM_SAMPLE_POINTS];
	int _recent_scans_tracker[36]; // divide 360 degrees into 36 bins
	int _recalibration_samples;
	int _num_samples_required;
	bool _contemplating_recalibration;
};




