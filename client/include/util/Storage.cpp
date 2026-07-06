#include "Storage.h"

#include <cstring>
#include <cstdio>
#include <vector>

std::string Storage::basePath;

bool Storage::Initialize() noexcept
{
    char buffer[MAX_PATH];
    const DWORD length = GetCurrentDirectory(MAX_PATH, buffer);
    if (length == 0 || length >= MAX_PATH) return false;

    basePath = buffer;
    basePath += "\\sampvoice\\resources\\";

    return true;
}

std::string Storage::GetResourcePath(const char* const fileName) noexcept
{
    if (basePath.empty()) return {};
    return basePath + fileName;
}

void Storage::ForEachFile(const std::function<void(const std::string&)>& callback) noexcept
{
    if (basePath.empty()) return;

    const std::string searchPath = basePath + "*";
    WIN32_FIND_DATA data;
    const HANDLE handle = FindFirstFile(searchPath.c_str(), &data);
    if (handle == INVALID_HANDLE_VALUE) return;

    do
    {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            callback(basePath + data.cFileName);
        }
    }
    while (FindNextFile(handle, &data));

    FindClose(handle);
}

std::vector<uint8_t> Storage::ReadFile(const std::string& path) noexcept
{
    std::vector<uint8_t> result;
    FILE* file = nullptr;

    if (fopen_s(&file, path.c_str(), "rb") != 0 || file == nullptr)
        return result;

    fseek(file, 0, SEEK_END);
    const long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size > 0)
    {
        result.resize(size);
        fread(result.data(), 1, size, file);
    }

    fclose(file);
    return result;
}
