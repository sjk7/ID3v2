#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <cstddef>
#include <bitset>
#include <array>
#include <cstdint>
#include <cassert>
#include <fstream>
#include "myUtils.hpp"

using namespace utils;

namespace ID3v2 {

using uchar = unsigned char;
template <size_t SZ> using bytearray = std::array<uchar, SZ>;

template <typename T> void swapEndian(T& val) {
    union U {
        T val;
        std::array<std::uint8_t, sizeof(T)> raw;
    } src, dst;

    src.val = val;
    std::reverse_copy(src.raw.begin(), src.raw.end(), dst.raw.begin());
    val = dst.val;
}

template <> void swapEndian<std::uint32_t>(std::uint32_t& value) {
    std::uint32_t tmp = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
    value = (tmp << 16) | (tmp >> 16);
}
static inline void openFile(const std::string& path, std::fstream& f) {}
#pragma pack(push, 1)
// clang-format off
struct TagHeader {
    char ID[3];             // ID3v2/file identifier   "ID3"
    uchar version[2];       // ID3v2 version           $03 00
    uchar flags;            // ID3v2 flags             %abc00000
    uint32_t sizeIndicator; // ID3v2 size              4* %0xxxxxxx
};
static constexpr inline auto TAG_HEADER_SIZE = sizeof(TagHeader);

struct TagExtendedHeader{
    uint32_t sizeBytesIndicator;    // $xx xx xx xx
    unsigned char flags[2];         // $xx xx
    uint32_t paddingSizeIndicator;  // $xx xx xx xx
};
static constexpr inline auto EXT_TAG_HEADER_SIZE = sizeof(TagExtendedHeader);

struct FrameHeader{
    // A tag must contain at least one frame.
    // A frame must be at least 1 byte big, excluding the header.
    char frameID;           // $xx xx xx xx (four characters)
    uint32_t sizeIndicator; // $xx xx xx xx
    std::byte Flags[2];     // $xx xx
};
// clang-format on

#pragma pack(pop)

struct TagHeaderEx : TagHeader {
    int validity = 0;
    bool hasUnsynchronisation = false;
    bool hasExtendedHeader = false;
    bool hasExperimental = true;
};

using size_type = typename std::string::size_type;
struct IDataReader {
    virtual size_t getSize() const = 0;
    virtual size_t getPos() const = 0;
    virtual std::string_view read(size_t&& bytes) = 0;
};

struct FileReader : IDataReader {
    virtual size_t getSize() const override { return m_size; }
    virtual size_t getPos() const override { return m_pos; }

    // will throw if you don't get all the data you asked for
    virtual std::string_view read(size_t&& nBytes) override {
        utils::file_read_some(m_buf, m_f, nBytes, fs::path{m_filePath});
        return std::string_view(m_buf.data(), m_buf.size());
    }

    // throws system_error on failure, hopefully with a meaningful message.
    FileReader(const std::string filePath) : m_filePath(filePath) {
        utils::file_open(m_f, filePath, std::ios::in | std::ios::binary, true);
        m_f.exceptions(0);
        m_f.clear();
    }

    private:
    std::fstream m_f;
    std::string m_buf;
    std::string m_filePath;
    size_type m_size = 0;
    size_type m_pos = 0;
};

static inline int verifyTag(TagHeaderEx& h) {
    std::string_view sv(h.ID, 3);
    if (h.version[0] != 3) return -1;
    if (sv != "ID3") return -2;
    std::bitset<8> f(h.flags);
    if (f.test(7)) h.hasUnsynchronisation = true;
    if (f.test(6)) h.hasExtendedHeader = true;
    if (f.test(5)) h.hasExperimental = true;
    f << 3;
    if (!f.none()) return -3; // flags that are reserved are set.
    return 0;
}
static inline TagHeaderEx parseTag(IDataReader& dr) {
    TagHeaderEx ret = {0};
    const auto data = dr.read(10);
    memcpy(&ret, data.data(), TAG_HEADER_SIZE);
    ret.validity = verifyTag(ret);
    return ret;
}

} // namespace ID3v2

using namespace std;
int tdd_file();
int main(int argc, char** argv) {

    return 0;
}
