#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <pigpio.h>
#include <iostream>

#define NUM_GPIO 32
#define MIN_RANGE 500
#define MAX_RANGE 2500

#define DEFAULT_CENTER_ANGLE 90
#define DEFAULT_CENTER_PULSE 1500

#define CENTER 0
#define RIGHT 1
#define LEFT 2

int gpio_pin;
int last_pos = 1500; // default center 

const std::string typical_angles[] = { "90", "180", "0" }; // indices match to #defs for CENTER/RIGHT/LEFT above
const std::string typical_pulse_widths[] = { "1500", "700", "2300" }; // indices match to #defs for CENTER/RIGHT/LEFT above

struct AngleMap
{
	int angle;
	int pulse_width;
};

void printWelcomeStatement()
{
	std::cout << "\n\nWelcome to the pigpio servo calibrator!\n\n";
	std::cout << "Here's the deal - pigpio servos are tricky.\n";
	std::cout << "Don't worry, we're gonna walk you through it...\n";
	std::cout << "All we're gonna do is ask you for some angles and\n";
	std::cout << "pulse widths to match those angles.\n\n";
	std::cout << "We have to do this because pigpio is amazing.\n";
	std::cout << "It allows you to control every gpio pin on\n";
	std::cout << "your rpi via PWM, which is awesome!\n";
	std::cout << "Usually the only supported pin is gpi 4!\n";
	std::cout << "Only issue is, it is based on a linux hack -\n";
	std::cout << "each servo is different and requires a different\n";
	std::cout << "mapping of pulse widths to angles.\n\n";
	std::cout << "We'll provide you with hints and guide you along the way.\n";
	std::cout << "Hopefully, we'll prevent you from destroying your servo.\n";
	std::cout << "Though, to be honest, we can't promise that.\n\n";
	std::cout << "Press CNTRL + C now if you're worried. Otherwise, let's go!\n\n";
}

void printExitStatement(AngleMap center_val, AngleMap right_val, AngleMap left_val)
{
	std::cout << "\nGreat job! Recentering servo for minimal wear...\n";
	std::cout << "\n\nCALIBRATION RESULTS FOR GPIO: (angle, pulse width) for pin " << gpio_pin << ":" << std::endl;
	std::cout << "\tRight\t(" << right_val.angle << " degrees, " << right_val.pulse_width << " duty cycle)\n";
	std::cout << "\tCenter\t(" << center_val.angle << " degrees, " << center_val.pulse_width << " duty cycle)\n";
	std::cout << "\tLeft\t(" << left_val.angle << " degrees, " << left_val.pulse_width << " duty cycle)\n";
	std::cout << "\nNext, you should test these calibration results by running the following command:\n";
	std::cout << "\n\n\t sudo ./test_servo " << gpio_pin << " " << right_val.angle << " " << right_val.pulse_width << " " << center_val.angle << " " << center_val.pulse_width << " " << left_val.angle << " " << left_val.pulse_width << std::endl;
	std::cout << " \n\nor you can call up an instance of a calibrated servo with the following code: TODO\n";
}

void stop(int is_error)
{
	std::cout << "\nFinishing...\n";

	gpioServo(gpio_pin, 0);
	gpioTerminate();

	if (is_error)
	{
		std::cout << "\nAborting the program because you told me to...\n";
		exit(is_error);
	}
	exit(0);
}

int rotate_servo(int last_pos, int new_pos)
{
	int pos = last_pos;
	int step = 1;

	// check valid range
	if (new_pos > MAX_RANGE || new_pos < MIN_RANGE)
	{
		std::cout << "Position of " << new_pos << " is invalid. Please select a range between " << MIN_RANGE << " and " << MAX_RANGE << ".\n";
		return last_pos;
	}

	// sweeping up or down?
	if (new_pos < pos)
	{
		step = -1;
	}

	// Perform the rotation
	while (pos != new_pos)
	{
		//printf("%d \n", pos);

		gpioServo(gpio_pin, pos);
		pos += step;
		time_sleep(0.001);
	}
	return new_pos;
}

std::string GetPositionString(int pos)
{
	std::string pos_string = "center";
	if (pos == LEFT)
	{
		pos_string = "leftmost";
	}
	else if (pos == RIGHT)
	{
		pos_string = "rightmost";
	}

	return pos_string;
}

AngleMap GetAcceptedVal(int position, AngleMap &accepted_val)
{
	int new_pos = accepted_val.pulse_width;
	std::cout << "new pos: " << new_pos << " last pos " << last_pos << std::endl;
	if (new_pos != last_pos)
	{
		last_pos = rotate_servo(last_pos, new_pos);
	}

	std::cout << "Do you accept the value of " << last_pos << " as the value of " << GetPositionString(position) << "? (y/n) -->";
	std::string resp = "n";
	std::cin >> resp;
	if (resp != "y")
	{
		std::cout << "Ok Enter a new value for position <" << GetPositionString(position) << ">: ";
		std::cin >> new_pos;
		accepted_val.pulse_width = new_pos;
		return GetAcceptedVal(position, accepted_val);
	}
	else
	{
		accepted_val.pulse_width = new_pos;
		return accepted_val;
	}
}

AngleMap PrintPrompt(int position)
{
	int angle = DEFAULT_CENTER_ANGLE;
	int pulse_width = DEFAULT_CENTER_PULSE;
	std::string pos_string = GetPositionString(position);

	std::cout << "Enter the desired angle for the " << pos_string << " position (typically around " << typical_angles[position] << " degrees) : ";
	std::cin >> angle;

	std::cout << "Enter a test pulse width for " << angle << " degrees (typically for " << typical_angles[position] << " degrees the pulse width is " << typical_pulse_widths[position] << ") : ";

	std::cin >> pulse_width;
	AngleMap angle_map= { angle, pulse_width };
	return angle_map;
}

AngleMap GetVal(int position)
{
	AngleMap angle_map = PrintPrompt(position);
	return GetAcceptedVal(position, angle_map);
}


int main(int argc, char *argv[])
{
	printWelcomeStatement();

	if (gpioInitialise() < 0)
	{
		std::cout << "Error initializing gpio - exiting\n";
		return -1;
	}

	gpioSetSignalFunc(SIGINT, stop);

	if (argc == 2)
	{
		gpio_pin = atoi(argv[1]);
	}

	gpioServo(gpio_pin, 1500);

	printf("Calibration routine starting - control C to stop.\n");
	AngleMap center_val = GetVal(CENTER);
	AngleMap right_val = GetVal(RIGHT);
	AngleMap left_val = GetVal(LEFT);

	printExitStatement(center_val, right_val, left_val);
	rotate_servo(left_val.pulse_width, center_val.pulse_width); // recenter the servo on our way out
	stop(0);

	return 0;
}

