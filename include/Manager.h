#pragma once

#include "Scanner.h"
#include <pigpio.h>   
#include <iostream>
#include <fstream>


#define IS_TRACKING 1
#define NOT_TRACKING 0
#define RECALIBRATION_DURATION 20 // seconds
#define SCANNING_INTERVAL 0.167 // scan rate is 6 hz

#define RECALIBRATION_PERCENT_THRESHOLD 90.0 // percent of samples that were valid during comtemplation period
#define RECALIBRATION_BIN_THRESHOLD 3 // number of bins populated during contemplation threshold

// rpi GPIO Pins
#define CALIBRATION_LED_PIN 18

#define LED_ON 1
#define LED_OFF 0

class Manager
{
public:
	Manager(RPlidarDriver * drv, Scanner *scanner);
	~Manager();

	/******** LIDAR CALIBRATION *************/
	// --------- methods ----------
	void CalibrateScanner();
	bool ShouldRecalibrate();
	bool ShouldStartContemplatingRecalibration(bool actively_tracking);
	void StartContemplatingRecalibration();
	void StopContemplatingRecalibration();
	void ContemplateRecalibration(ScanResult scan);
	// --------- variables ----------
	double calibration_values[NUM_SAMPLE_POINTS];
	int _recent_scans_tracker[36]; // divide 360 degrees into 36 bins
	int _recalibration_samples;
	int _num_samples_required;
	bool _contemplating_recalibration;


	/******** LIGHTING *******************/
	// --------- methods ----------
	void InitializeLEDs();
	void ToggleLED(int pin, int state);
	//--------- variables ----------
	int _calibration_LED_pin;

	/******** STATE MACHINE *************/
	// --------- methods ----------
	void ParseResult(ScanResult scan);
	bool DetectingSomething(ScanResult scan);
	// --------- variables ----------
	int _manager_state;
	RPlidarDriver * _lidar_driver;
	Scanner * _scanner;

	/********** HEARTBEAT *************/
	// --------- methods ----------
	void StartHeartbeat();
	void Heartbeat();
	void StopHeartbeat();
	// --------- variables ----------
	std::ofstream _heartbeat_fh;
};




