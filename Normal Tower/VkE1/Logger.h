#ifndef	LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

#define COUT_LOG
#define SAVE_LOG_ALWAYS

#ifdef COUT_LOG
#include <iostream>
#endif

class Logger
{
	std::ofstream file;
	std::string name;

public:
	void Start(const char* _logFilename)
	{
		if (file.is_open())
			file.close();
		file = std::ofstream(_logFilename);
		name = _logFilename;
	}
	void Set(const char* _logFilename)
	{
		if (file.is_open())
			file.close();
		file = std::ofstream(_logFilename, std::fstream::app);
		name = _logFilename;
	}
	void Close()
	{
		file.close();
	}

	template <typename T>
	Logger &operator<<(const T &_data)
	{
		file << _data;

#ifdef SAVE_LOG_ALWAYS
		Close();
		Set(name.c_str());
#endif

#ifdef COUT_LOG
		std::cout << _data;
#endif
		return *this;
	}

	~Logger()
	{
		file.close();
	}
};

#endif
