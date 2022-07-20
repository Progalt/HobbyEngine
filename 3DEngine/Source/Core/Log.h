#pragma once

#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; 
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); 
}

class Log
{
public:

	template<typename ... Args>
	static void Info(const std::string& moduleName, const std::string& msg, Args... args)
	{
		Print("Info", moduleName, msg, args...);
	}

	template<typename ... Args>
	static void Warn(const std::string& moduleName, const std::string& msg, Args... args)
	{
		Print("Warn", moduleName, msg, args...);
	}

	template<typename ... Args>
	static void Error(const std::string& moduleName, const std::string& msg, Args... args)
	{
		Print("Error", moduleName, msg, args...);

		std::string err = string_format(msg, args...);

		throw std::runtime_error(err);
	}



private:

	template<typename ... Args>
	static void Print(const std::string& sev, const std::string& moduleName, std::string fmt, Args... args)
	{
		std::string baseStr = string_format(fmt, args...);

		std::string finalPrint = string_format("[%s][%s] -> %s\n", sev.c_str(), moduleName.c_str(), baseStr.c_str());
		
		printf("%s", finalPrint.c_str());
	}

	
};