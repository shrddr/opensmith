#pragma once
#include <string>
#include <fstream>

class Logger
{
public:
	Logger(const char* fileName);
	~Logger();
	void info(std::string text);
private:
	std::ofstream f;
};

