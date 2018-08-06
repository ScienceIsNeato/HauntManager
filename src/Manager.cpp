#include "../include/Manager.h"
#include <cmath>

Manager::Manager()
{
}

Manager::~Manager()
{
}

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
