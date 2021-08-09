#ifndef __CONFIG_PARSER_H__
#define __CONFIG_PARSER_H__

#include "../include/Ghoul.h"
#include <vector>

class ConfigParser
{
	private:

	public:
		static bool ReadConfig(std::vector<Ghoul*> &ghouls);
		static void PrintConfig(Ghoul* ghoul);
};

#endif // __CONFIG_PARSER_H__
