#include "../include/Scanner.h"

Scanner::Scanner()
{
}

Scanner::~Scanner()
{
}

bool Scanner::Initialize(RPlidarDriver * drv, const char * com_path, const char * baud_rate)
{
	const char * opt_com_path = NULL;
	_u32         baudrateArray[2] = { 115200, 256000 };
	_u32         opt_com_baudrate = 0;
	u_result     op_result;

	bool useArgcBaudrate = false;

	printf("Ultra simple LIDAR data grabber for RPLIDAR.\n"
		"Version: 1.6.1\n");

	// read serial port from the command line...
	if (com_path) opt_com_path = com_path; // or set to a fixed value: e.g. "com3" 

	// read baud rate from the command line if specified...
	if (baud_rate)
	{
		opt_com_baudrate = strtoul(baud_rate, NULL, 10);
		useArgcBaudrate = true;
	}

	if (!opt_com_path) {
#ifdef _WIN32
		// use default com port
		opt_com_path = "\\\\.\\com3";
#else
		opt_com_path = "/dev/ttyUSB0";
#endif
	}

	rplidar_response_device_info_t devinfo;
	bool connectSuccess = false;
	// make connection...
	if (useArgcBaudrate)
	{
		if (!drv)
			drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
		if (IS_OK(drv->connect(opt_com_path, opt_com_baudrate)))
		{
			op_result = drv->getDeviceInfo(devinfo);

			if (IS_OK(op_result))
			{
				connectSuccess = true;
			}
			else
			{
				delete drv;
				drv = NULL;
			}
		}
	}
	else
	{
		size_t baudRateArraySize = (sizeof(baudrateArray)) / (sizeof(baudrateArray[0]));
		for (size_t i = 0; i < baudRateArraySize; ++i)
		{
			if (!drv)
				drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
			if (IS_OK(drv->connect(opt_com_path, baudrateArray[i])))
			{
				op_result = drv->getDeviceInfo(devinfo);

				if (IS_OK(op_result))
				{
					connectSuccess = true;
					break;
				}
				else
				{
					delete drv;
					drv = NULL;
				}
			}
		}
	}
	if (!connectSuccess) {

		fprintf(stderr, "Error, cannot bind to the specified serial port %s.\n"
			, opt_com_path);
		return false;
	}

	// print out the device serial number, firmware and hardware version number..
	printf("RPLIDAR S/N: ");
	for (int pos = 0; pos < 16; ++pos)
	{
		printf("%02X", devinfo.serialnum[pos]);
	}

	printf("\n"
		"Firmware Ver: %d.%02d\n"
		"Hardware Rev: %d\n"
		, devinfo.firmware_version >> 8
		, devinfo.firmware_version & 0xFF
		, (int)devinfo.hardware_version);

	return true;
}

bool Scanner::CheckRPLIDARHealth(RPlidarDriver * drv)
{
	u_result     op_result;
	rplidar_response_device_health_t healthinfo;

	op_result = drv->getHealth(healthinfo);
	if (IS_OK(op_result)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
		printf("RPLidar health status : %d\n", healthinfo.status);
		if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
			fprintf(stderr, "Error, rplidar internal error detected. Please reboot the device to retry.\n");
			// enable the following code if you want rplidar to be reboot by software
			// drv->reset();
			return false;
		}
		else {
			return true;
		}

	}
	else {
		fprintf(stderr, "Error, cannot retrieve the lidar health code: %x\n", op_result);
		return false;
	}
}

double Scanner::CorrectAngle(double angle)
{
	/* For some reason, the scanner returns results like this

				180
				|
				|
				|
	90---------------------270
				|
				|
				|
				0

	We want it like this

				90
				|
				|
				|
	180---------------------0
				|
				|
				|
				270

	So, first we reverse the direction of the angle, then we rotate by 90 degrees */
	double retval = 360 - angle;
	retval -= 90;
	return retval;
}

void Scanner::Close(RPlidarDriver * drv)
{
	RPlidarDriver::DisposeDriver(drv);
	drv = NULL;
}

void Scanner::Calibrate(RPlidarDriver * drv, int num_samples, double (&calibration_results) [NUM_SAMPLE_POINTS])
{
	int max_attempts = 500;
	int good_samples = 0;
	int attempts = 0;
	double smoothed_cal_vals[NUM_SAMPLE_POINTS];


	for (int i = 0; i < NUM_SAMPLE_POINTS; i++)
	{
		calibration_results[i] = DEFAULT_CALIBRATION_VALUE;
		smoothed_cal_vals[i] = 0;
	}

	for (int i = 100; i > 0; i--)
	{
		std::cout << "Calibrating in " << i << " ...\n";
		//std::this_thread::sleep_for(std::chrono::seconds(1)); This is broken
	}

	std::cout << "Calibration countdown! -- " << num_samples << std::endl;

	while((attempts < max_attempts) && (good_samples < num_samples))
	{
		rplidar_response_measurement_node_t nodes[NUM_SAMPLE_POINTS];
		size_t   count = _countof(nodes);
		u_result op_result = drv->grabScanData(nodes, count);

		if (IS_OK(op_result)) 
		{
			drv->ascendScanData(nodes, count);
			std::cout << num_samples - good_samples << "\n" << std::flush;
			
			for (int pos = 0; pos < (int)count; ++pos)
			{
				double dist = nodes[pos].distance_q2 / 4.0f;
				if (dist > 0)
				{
					if (dist < calibration_results[pos])
					{
						calibration_results[pos] = dist;
					}
				}
			}
			good_samples++;
		}
		else
		{
			std::cout << "attempt " << attempts << " Failed.\n" << std::flush;
		}
		attempts++;
	}

	std::cout << "Calibration gathered " << good_samples << " good samples out of " << attempts << "attempts.\n" << std::flush;

	// Multiply the results by the scale factor
	int bad_samples = 0;
	for (int i = 0; i < NUM_SAMPLE_POINTS; i++)
	{
		if (calibration_results[i] == DEFAULT_CALIBRATION_VALUE)
		{
			bad_samples++;
		}
	}
	std::cout << "Calibration found " << NUM_SAMPLE_POINTS - bad_samples << " valid samples out of " << NUM_SAMPLE_POINTS << " total collected.\n" << std::flush;
	SmoothCalibrationResults(calibration_results, smoothed_cal_vals, .98);
}

void Scanner::SmoothCalibrationResults(double(&calibration_results)[NUM_SAMPLE_POINTS], double(&smoothed_cal_vals)[NUM_SAMPLE_POINTS], double scale_factor)
{
	int range = 5;
	int adjusted = 0;
	double adjustment_sum = 0.0;
	for (int i = 0; i < NUM_SAMPLE_POINTS; i++)
	{
		double val = calibration_results[i];
		for (int index = range * -1; index <= range; index++)
		{
			int relative_index = i + index;
			if ( (relative_index > 0) && (relative_index < NUM_SAMPLE_POINTS) && (calibration_results[relative_index] < val))
			{
				val = calibration_results[relative_index];
			}
		}

		smoothed_cal_vals[i] = val * scale_factor;

		if (val != calibration_results[i])
		{
			adjusted++;
			adjustment_sum += calibration_results[i] - val;
		}

	}
	std::cout << "Smoothing complete. " << adjusted << " points have been adjusted. Avg ajdustment: " << adjustment_sum/double(adjusted) << "\n" << std::flush;
	
	// Assign the smoothed values as the final calibration results to be passed back up the stack
	for (int i = 0; i < NUM_SAMPLE_POINTS; i++)
	{
		calibration_results[i] = smoothed_cal_vals[i];
	}
}

bool Scanner::Start(RPlidarDriver *drv, const char * com_path, const char * baud_rate)
{
	if (!(Initialize(drv, com_path, baud_rate)) || (!CheckRPLIDARHealth(drv)))
	{
		return false;
	}
	drv->startMotor();
	drv->startScan(0, 1);

	return true;
}

void Scanner::Stop(RPlidarDriver *drv)
{
	printf("\nStopping the LIDAR scanner driver...");
	drv->stop();
	printf("\nStopping the LIDAR scanner motor...");
	drv->stopMotor();
}

ScanResult Scanner::Scan(RPlidarDriver * drv, double(calibration_values)[NUM_SAMPLE_POINTS])
{
	u_result     op_result;
	ScanResult ret_val;
	ret_val.valid = false;

	// fetch result and print it out...
	rplidar_response_measurement_node_t nodes[NUM_SAMPLE_POINTS];
	size_t count = _countof(nodes);
	op_result = drv->grabScanData(nodes, count);

	if (IS_OK(op_result))
	{
		drv->ascendScanData(nodes, count);
		for (int pos = 0; pos < (int)count; ++pos)
		{
			double dist = nodes[pos].distance_q2 / 4.0f;
			double angle = (nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f;
			if (!(ShouldProcess(angle, dist)))
			{
				// point is in a dead zone, continue
				continue;
			}
			if ((dist > 0) &&
				(dist < calibration_values[pos]) &&
				(nodes[pos].sync_quality > 40) )
			{
				ret_val.closest_distance = dist;
				ret_val.closest_angle = (nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f;
				ret_val.closest_index = pos;
				ret_val.valid = true;
			}
		}
	}	
	return ret_val;
}

// Return false if point is in a dead zone
bool Scanner::ShouldProcess(double angle, double distance)
{
	bool should_process = true;
	for (auto & dead_zone : _dead_zones) 
	{
		if ((distance >= dead_zone.start_distance) &&
			(distance <= dead_zone.end_distance) &&
			(angle >= dead_zone.start_angle) &&
			(angle <= dead_zone.end_angle))
		{
			should_process = false;
		}
	}
	return should_process;
}

// Create a region for which measurements should be ignored
void Scanner::AddDeadZone(DeadZone dead_zone)
{
	// Basic validation
	if (dead_zone.start_angle > dead_zone.end_angle)
	{
		std::cout << "\nInvalid dead zone. Start angle must be smaller than end angle.\n";
		return;
	}

	if (dead_zone.start_distance > dead_zone.end_distance)
	{
		std::cout << "\nInvalid dead zone. Start distance must be smaller than end distance.\n";
		return;
	}

	if ((dead_zone.start_angle > 360.0) || (dead_zone.start_angle < 0))
	{
		std::cout << "\nInvalid dead zone. Start angle must be between 0 and 360 degrees.\n";
		return;
	}

	if ((dead_zone.end_angle > 360.0) || (dead_zone.end_angle < 0))
	{
		std::cout << "\nInvalid dead zone. End angle must be between 0 and 360 degrees.\n";
		return;
	}

	// Valid dead zone
	_dead_zones.push_back(dead_zone);
}

