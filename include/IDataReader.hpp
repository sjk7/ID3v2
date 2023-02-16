#pragma once
// IDataReader.hpp
#include <istream>
#include <string>

namespace my {
using size_type = typename std::string::size_type;
struct IDataReader {
    virtual size_t getSize(bool refresh = false) const noexcept = 0;
    virtual size_t getPos(bool refresh = false) const noexcept = 0;
    virtual std::streamoff seek(
        std::streamoff pos, bool& ok, std::ios::seekdir dir = std::ios::beg)
        = 0;
    virtual std::string_view read(size_t&& bytes) = 0;
    virtual size_t readInto(std::string&, const size_t nBytes) = 0;
    virtual const std::string& data() const noexcept = 0;
};
} // namespace my
