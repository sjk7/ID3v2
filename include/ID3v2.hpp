#pragma once
#include "FileDataReader.hpp"
#include "myUtils.hpp"

namespace ID3v2
{
static constexpr int OK = 0;

using uchar = unsigned char;
template <size_t SZ> using bytearray = std::array<uchar, SZ>;

template <typename T> void swapEndian(T &val)
{
    union U {
        T val;
        std::array<std::uint8_t, sizeof(T)> raw;
    } src, dst;

    src.val = val;
    std::reverse_copy(src.raw.begin(), src.raw.end(), dst.raw.begin());
    val = dst.val;
}

template <> void swapEndian<std::uint32_t>(std::uint32_t &value)
{
    std::uint32_t tmp = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
    value = (tmp << 16) | (tmp >> 16);
}
static inline void openFile(const std::string &path, std::fstream &f)
{
}
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

struct TagHeaderEx : TagHeader
{
    int validity = 0;
    bool hasUnsynchronisation = false;
    bool hasExtendedHeader = false;
    bool hasExperimental = true;
    uint32_t dataSizeInBytes = 0;
    uint32_t totalSizeInBytes() const noexcept
    {
        return dataSizeInBytes > 0 ? dataSizeInBytes + 10 : 0;
    }
};

static inline int verifyTag(TagHeaderEx &h)
{
    std::string_view sv(h.ID, 3);
    if (h.version[0] != 3)
        return -1;
    if (sv != "ID3")
        return -2;
    if (h.sizeIndicator == 0)
        return -4;
    std::bitset<8> f(h.flags);
    if (f.test(7))
        h.hasUnsynchronisation = true;
    if (f.test(6))
        h.hasExtendedHeader = true;
    if (f.test(5))
        h.hasExperimental = true;
    f = f << 3;
    if (!f.none())
        return -3; // flags that are reserved are set.
    return 0;
}
static inline uint32_t decodeSynchSafe(uint32_t size)
{
    size = (size & 0x0000007F) | ((size & 0x00007F00) >> 1) | ((size & 0x007F0000) >> 2) | ((size & 0x7F000000) >> 3);
    return size;
}
static inline uint32_t getTagSize(uint32_t sz)
{
    swapEndian(sz);
    sz = decodeSynchSafe(sz);
    return sz;
}
static inline TagHeaderEx parseTag(my::IDataReader &dr)
{
    TagHeaderEx ret = {0};
    const auto data = dr.read(10);
    if (data.size() == 10)
    {
        memcpy(&ret, data.data(), TAG_HEADER_SIZE);
        ret.validity = verifyTag(ret);
        ret.dataSizeInBytes = getTagSize(ret.sizeIndicator);
    }
    return ret;
}

} // namespace ID3v2
