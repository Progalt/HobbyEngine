
#include "FileSystem.h"

#include <fstream>

std::vector<int8_t> FileSystem::ReadBytes(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);

    std::vector<int8_t> bytes(
        (std::istreambuf_iterator<char>(input)),
        (std::istreambuf_iterator<char>()));

    input.close();

    return bytes;
}