#include "Storage.h"

#include <cstring>
#include <cstdio>
#include <vector>

#include "Logger.h"

std::string Storage::basePath;

bool Storage::Initialize() noexcept
{
    if (!Storage::basePath.empty())
        return true;

    char modulePath[MAX_PATH] {};
    const DWORD length = GetModuleFileNameA(nullptr, modulePath, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) return false;

    // modulePath points at the .asi/.exe; strip the file name to get the directory.
    char* const lastSlash = std::strrchr(modulePath, '\\');
    if (lastSlash == nullptr) return false;

    *lastSlash = '\0';

    Storage::basePath = modulePath;
    Storage::basePath += "\\sampvoice\\resources\\";

    Logger::LogToFile("[sv:inf:storage] : resource base path '%s'", Storage::basePath.c_str());

    return true;
}

std::string Storage::GetResourcePath(const char* const fileName) noexcept
{
    if (basePath.empty()) return {};
    return basePath + fileName;
}

std::string Storage::GetLanguagesPath() noexcept
{
    if (basePath.empty()) return {};

    // resources/ -> languages/  (siblings under <dir>\sampvoice\)
    const std::string baseDir = basePath.substr(0, basePath.size() - std::strlen("resources\\"));
    return baseDir + "languages\\";
}

void Storage::ForEachFile(const std::function<void(const std::string&)>& callback) noexcept
{
    if (basePath.empty()) return;

    const std::string searchPath = basePath + "*";
    WIN32_FIND_DATAA data;
    const HANDLE handle = FindFirstFileA(searchPath.c_str(), &data);
    if (handle == INVALID_HANDLE_VALUE) return;

    do
    {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            callback(basePath + data.cFileName);
        }
    }
    while (FindNextFileA(handle, &data));

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
