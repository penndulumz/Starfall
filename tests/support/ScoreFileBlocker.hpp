#pragma once

#include <filesystem>
#include <stdexcept>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace starfall::testing {

// Hold a real Windows reader that allows other readers but forbids replacement.
// This exercises the actual filesystem failure without a production test hook.
class ScoreFileBlocker {
public:
    explicit ScoreFileBlocker(const std::filesystem::path& file)
        : handle(CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)) {
        if (handle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Could not establish score replacement failure scenario");
        }
    }
    ~ScoreFileBlocker() { CloseHandle(handle); }
    ScoreFileBlocker(const ScoreFileBlocker&) = delete;
    ScoreFileBlocker& operator=(const ScoreFileBlocker&) = delete;

private:
    HANDLE handle;
};

} // namespace starfall::testing
#endif
