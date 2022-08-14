
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

std::string FileSystem::GetDirectory(const std::string& path)
{
    size_t pos = path.find_last_of("\\/");
    return (std::string::npos == pos)
        ? ""
        : path.substr(0, pos);
}