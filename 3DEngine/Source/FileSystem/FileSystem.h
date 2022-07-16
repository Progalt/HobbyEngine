#pragma once


#include <filesystem>

#include <cstdint>

class FileSystem
{
public:

	static std::vector<int8_t> ReadBytes(const std::filesystem::path& path);

private:
};