#pragma once

#include <string>
#include <memory>
#include <functional>
#include <vector>

#include <Windows.h>

class Storage {

    Storage() = delete;
    ~Storage() = delete;
    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    Storage& operator=(const Storage&) = delete;
    Storage& operator=(Storage&&) = delete;

public:

    static bool Initialize() noexcept;
    static std::string GetResourcePath(const char* fileName) noexcept;
    static void ForEachFile(const std::function<void(const std::string&)>& callback) noexcept;
    static std::vector<uint8_t> ReadFile(const std::string& path) noexcept;

private:

    static std::string basePath;

};
