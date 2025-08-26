#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sstream>

inline int stringToInt(const std::string &s)
{
	std::istringstream iss(s);
	int value;
	iss >> value;
	return value;
}

#endif
