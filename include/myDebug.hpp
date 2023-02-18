#pragma once
// myTrace.hpp
#include <iostream>
#include <sstream>
#ifdef _MSC_VER
#define NOMINMAX
#define VC_EXTRALEAN
#include <Windows.h> // OutputDebugString
// #include <atlbase.h>
#endif

#define S1(x) #x
#define S2(x) S1(x)
#define S3(x) x

#define MYLOCATION __FILE__ " : " S3(__func__) " : " S2(__LINE__)

struct MyTraceFlags {
    static inline constexpr unsigned int to_cout = 2;
    static inline constexpr unsigned int to_cerr = 4;
    static inline constexpr unsigned int to_debug_window = 8;
    static inline constexpr unsigned int defaults = to_cerr | to_debug_window;
};

namespace detail {

template <typename... Args> void doPrint(std::ostream& out, Args&&... args) {
    ((out << ' ' << std::forward<Args>(args)), ...);
}

} // namespace detail

template <typename... Args>
static inline void MyTrace(const char* const file, const int line,
    const unsigned int flags, Args&&... args) {
    using std::cerr;
    using std::cout;
    using std::endl;
    // ATLTRACE("ffs");
    if (flags & MyTraceFlags::to_cout) {
        cout << file << ":" << line << endl;
        detail::doPrint(cout, std::forward<Args>(args)...);
    }
    if (flags & MyTraceFlags::to_cerr) {
        cerr << file << ":" << line << endl;
        detail::doPrint(cout, std::forward<Args>(args)...);
    }
    if (flags & MyTraceFlags::to_debug_window) {
        std::stringstream ss;
        detail::doPrint(ss, std::forward<Args>(args)...);
        std::stringstream fl;
        fl << '\n'
           << file << "(" << line << ")"
           << " : " << endl;
        ::OutputDebugString(fl.str().c_str());
        ::OutputDebugStringA(ss.str().c_str());
    }
}

#define MYTRACE(flags, ...) MyTrace(__FILE__, __LINE__, flags, __VA_ARGS__);
#define MYASSERT(__FILE__, __LINE__, arg)
