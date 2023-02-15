#pragma once
// mySystemError.hpp
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace my {
namespace os {
    static inline int SysErrorCode() {
        return static_cast<int>(GetLastError());
    }

    // Returns the last Win32 error, in string format. Returns an empty string
    // if there is no error.
    std::string SysErrorString(int e = -1) {
        // Get the error message ID, if any.
        DWORD errorMessageID = e == 0 ? GetLastError() : static_cast<DWORD>(e);
        if (errorMessageID == 0) {
            return std::string(); // No error message has been recorded
        }

        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);

        return message;
    }
    
#else
static inline int SysErrorCode() {
    return errno;
}

static inline std::string SysErrorString(int e = -1) {
    e = e == 0 ? errno : e;
    return strerror(e);
}

#endif
}
}
