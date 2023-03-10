// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

#pragma once
// IDataReader.hpp
#include <istream>
#include <string>

namespace my {
using size_type = uintmax_t;
struct IDataReader {
    virtual std::streamsize getSize(bool refresh = false) const noexcept = 0;
    virtual std::streampos getPos(bool refresh = false) const noexcept = 0;
    virtual std::streamoff seek(
        std::streamoff pos, std::ios::seekdir dir = std::ios::beg)
        = 0;
    virtual std::string_view read(size_t&& bytes) = 0;
    virtual std::streamsize readInto(std::string&, const size_t nBytes) = 0;
    virtual std::streamsize readInto(void* dest, size_t nBytes) noexcept = 0;

    virtual const std::string& data() const noexcept = 0;
};
} // namespace my
