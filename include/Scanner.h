#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "../include/rplidar.h" //RPLIDAR standard sdk, all-in-one header

#define NUM_SAMPLE_POINTS 8192
#define CALIBRATION_PNTS 50 // should make injectable
#define CALIBRATION_SCALE_FACTOR 0.98
#define DEFAULT_CALIBRATION_VALUE 15000.0

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif


using namespace rp::standalone::rplidar;

struct ScanResult
{
	double closest_angle;
	double closest_distance;
	int closest_index;
	bool valid;
};

class Scanner
{
public:
	Scanner();
	~Scanner();

	// Check if the data coming from the lidar is valid
	bool CheckRPLIDARHealth(RPlidarDriver * drv);

	// Cleanup
	void Close(RPlidarDriver * drv);

	bool Start(RPlidarDriver * drv, int argc, const char * argv[]);
	void Stop(RPlidarDriver * drv);


	// Set up the rplidar driver and initialize
	bool Initialize(RPlidarDriver * drv, int argc, const char * argv[]);

	// Get raw data of surroundings for a period of time and average them per angle measurement
	void Calibrate(RPlidarDriver * drv, int num_samples, double (&calibration_results) [NUM_SAMPLE_POINTS]);

	// Take raw calibration results and manipulate them to avoid false positives during detection
	void SmoothCalibrationResults(double(&calibration_results)[NUM_SAMPLE_POINTS], double(&smoothed_cal_vals)[NUM_SAMPLE_POINTS], double scale_factor);

	// Get raw values for a single pass
	ScanResult Scan(RPlidarDriver * drv, double(calibration_values)[NUM_SAMPLE_POINTS]);
};

