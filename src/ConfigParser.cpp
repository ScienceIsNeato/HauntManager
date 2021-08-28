#include "../include/ConfigParser.h"

#include <fstream>

bool ConfigParser::ReadConfig(std::vector<Ghoul*> &ghouls)
{
	// TODO: I tried to load an old config and this segfaulted. You should fix that.

	std::cout << "Reading servo configurations...\n";
	std::ifstream infile("servo_config.conf");

	if(!infile)
	{
		std::cout << "Couldn't open `servo_config.conf`. Did you copy over the template like in the README?\n";
		return false;
	}

	ServoConfig *current_config = nullptr;
	Ghoul *current_ghoul = nullptr;

	std::string line;
	std::string delimiter = ":";
	std::size_t delim_pos;
	std::string key;
	std::string val;
	size_t first;
	size_t last;

	while(getline (infile, line))
	{
		// Skip blank lines
		if(line.length() == 0)
		{
			// Blank line - ignore
			continue;
		}
		else if(line.substr(0,1) == "#")
		{
			// Comment - ignore
			continue;
		}

		// Make sure there's a colon on the line
		delim_pos = line.find(delimiter);
		if(delim_pos == std::string::npos)
		{
			std::cout << "Error - no `:` found on this line of config: " << line << ".\n";
			return false;
		}

		// Make sure delimiter isn't the start or end of the line
		if(delim_pos == 0 || delim_pos == line.length())
		{
			std::cout << "Error - config line can't start or end with a colon.\n";
			return false;
		}

		// Grab key-value pair
		key = line.substr(0, delim_pos);
		val = line.substr(delim_pos + 1);

		// Trim any whitespace
		first = key.find_first_not_of(' ');
		last = key.find_last_not_of(' ');
		key = key.substr(first, (last-first+1));

		first = val.find_first_not_of(' ');
		last = val.find_last_not_of(' ');
		val = val.substr(first, (last-first+1));

		if(key == "GHOUL")
		{
			current_ghoul = new Ghoul(val);
		}
		else if(key == "GHOUL_END")
		{
			ghouls.push_back(current_ghoul);
			current_config = nullptr;
			current_ghoul = nullptr;
		}
		else if(key == "SERVO_NAME")
		{
			current_config = new ServoConfig();
			current_config->name = val;
		}
		else if(key == "SERVO_END")
		{
			if(current_config->is_horizontal)
			{
				current_ghoul->SetHorizServo(current_config);
			}
			else
			{
				current_ghoul->SetVertServo(current_config);
			}
			current_config = nullptr;
		}
		else if(key == "gpio_pin")
		{
			current_config->gpio_pin = atoi(val.c_str());
		}
		else if(key == "min.angle")
		{
			current_config->angle_maps.min_map.angle = atoi(val.c_str());
		}
		else if(key == "center.angle")
		{
			current_config->angle_maps.center_map.angle = atoi(val.c_str());
		}
		else if(key == "max.angle")
		{
			current_config->angle_maps.max_map.angle = atoi(val.c_str());
		}
		else if(key == "min.pulse_width")
		{
			current_config->angle_maps.min_map.pulse_width = atoi(val.c_str());
		}
		else if(key == "center.pulse_width")
		{
			current_config->angle_maps.center_map.pulse_width = atoi(val.c_str());
		}
		else if(key == "max.pulse_width")
		{
			current_config->angle_maps.max_map.pulse_width = atoi(val.c_str());
		}
		else if(key == "offsetAngle")
		{
			current_config->offsets.offsetAngle = atoi(val.c_str());
		}
		else if(key == "offsetX")
		{
			current_config->offsets.offsetX = atoi(val.c_str());
		}
		else if(key == "offsetY")
		{
			current_config->offsets.offsetY = atoi(val.c_str());
		}
		else if(key == "type")
		{
			if (val == "horizontal")
			{
				current_config->is_horizontal = true;
			}
			else
			{
				current_config->is_horizontal = false;
			}
		}
		else if(key == "eyes_pin")
		{
			current_ghoul->SetEyes(atoi(val.c_str()));
		}
		else
		{
			std::cout << "Unknown KEY, VALUE pair: " << key << ", " << val;
			return false;
		}
	}

	std::cout << "Parsing complete - loaded " << ghouls.size() << " ghoul configurations.\n";

	return true;
}

void ConfigParser::PrintConfig(Ghoul* ghoul)
{
	std::cout << "\nGHOUL NAME:  " << ghoul->GetName();

	std::cout << "\n  HORIZONTAL SERVO:     " << ghoul->GetHorizServoConfig()->name;
	std::cout << "\n    gpio_pin:           " << ghoul->GetHorizServoConfig()->gpio_pin;
	std::cout << "\n    min.angle:          " << ghoul->GetHorizServoConfig()->angle_maps.min_map.angle;
	std::cout << "\n    center.angle:       " << ghoul->GetHorizServoConfig()->angle_maps.center_map.angle;
	std::cout << "\n    max.angle:          " << ghoul->GetHorizServoConfig()->angle_maps.max_map.angle;
	std::cout << "\n    min.pulse_width:    " << ghoul->GetHorizServoConfig()->angle_maps.min_map.pulse_width;
	std::cout << "\n    center.pulse_width: " << ghoul->GetHorizServoConfig()->angle_maps.center_map.pulse_width;
	std::cout << "\n    max.pulse_width:    " << ghoul->GetHorizServoConfig()->angle_maps.max_map.pulse_width;
	std::cout << "\n    offsetAngle:        " << ghoul->GetHorizServoConfig()->offsets.offsetAngle;
	std::cout << "\n    offsetX:            " << ghoul->GetHorizServoConfig()->offsets.offsetX;
	std::cout << "\n    offsetY:            " << ghoul->GetHorizServoConfig()->offsets.offsetY << std::endl;

	std::cout << "\n  VERTICAL SERVO:       " << ghoul->GetVertServoConfig()->name;
	std::cout << "\n    gpio_pin:           " << ghoul->GetVertServoConfig()->gpio_pin;
	std::cout << "\n    asleep.angle:       " << ghoul->GetVertServoConfig()->angle_maps.min_map.angle;
	std::cout << "\n    center.angle:       " << ghoul->GetVertServoConfig()->angle_maps.center_map.angle;
	std::cout << "\n    awake.angle:        " << ghoul->GetVertServoConfig()->angle_maps.max_map.angle;
	std::cout << "\n    asleep.pulse_width: " << ghoul->GetVertServoConfig()->angle_maps.min_map.pulse_width;
	std::cout << "\n    center.pulse_width: " << ghoul->GetVertServoConfig()->angle_maps.center_map.pulse_width;
	std::cout << "\n    awake_pulse_width:  " << ghoul->GetVertServoConfig()->angle_maps.max_map.pulse_width << std::endl;

	std::cout << "\n  Eyes LED Pin:         " << ghoul->GetEyes() << std::endl;;
}