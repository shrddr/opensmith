#include "Logger.h"

Logger::Logger(const char* fileName)
{
	f.open(fileName);
}

Logger::~Logger()
{
	f.close();
}

void Logger::info(std::string text)
{
	f << text << std::endl;
}
