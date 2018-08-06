#pragma once

#define IS_TRACKING 1
#define NOT_TRACKING 0

class Manager
{
public:
	Manager();
	~Manager();

	bool ShouldRecalibrate(int &current_state, int &counter, double angle, double first_angle_detected);

	int manager_state;
	double last_tracking_angle;
	int continuous_tracking_counter;
};

