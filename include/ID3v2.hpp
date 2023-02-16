// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

#pragma once
#include "FileDataReader.hpp"
#include "myUtils.hpp"
#include <bitset>

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

static_assert(std::is_pod_v<TagHeader>);

struct TagExtendedHeader{
    uint32_t sizeBytesIndicator;    // $xx xx xx xx
    unsigned char flags[2];         // $xx xx
    uint32_t paddingSizeIndicator;  // $xx xx xx xx
};
static constexpr inline auto EXT_TAG_HEADER_SIZE = sizeof(TagExtendedHeader);
static_assert(std::is_pod_v<TagExtendedHeader>);

struct FrameHeader{
    // A tag must contain at least one frame.
    // A frame must be at least 1 byte big, excluding the header.
    char frameID;           // $xx xx xx xx (four characters)
    uint32_t sizeIndicator; // $xx xx xx xx
    std::byte Flags[2];     // $xx xx
};
// clang-format on
static_assert(std::is_pod_v<FrameHeader>);

struct ID3v1Tag {
    char tag[3]; // should be "TAG"
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    unsigned char genre; // should be an index into a pre-defined list of genres
};
static_assert(std::is_pod_v<ID3v1Tag>);
static_assert(sizeof(ID3v1Tag) == 128);
static constexpr int16_t ID3V1_TAG_SIZE = 128;
#pragma pack(pop)

enum class verifyTagResult {
    BadBase = -1,
    OK = 1,
    BadVersion = BadBase - 1,
    BadID = BadBase - 2,
    BadSizeIndicator = BadBase - 3,
    BadReservedFlags = BadBase - 4
};

struct TagHeaderEx : TagHeader {
    ID3v1Tag v1Tag = {};
    verifyTagResult validity = verifyTagResult::OK;
    bool hasUnsynchronisation = false;
    bool hasExtendedHeader = false;
    bool hasExperimental = true;
    uint32_t dataSizeInBytes = 0;
    uint32_t totalSizeInBytes() const noexcept {
        return dataSizeInBytes > 0 ? dataSizeInBytes + sizeof(TagHeader) : 0;
    }
    bool hasv1Tag() const {
        std::string_view s{v1Tag.tag};
        return s == "TAG";
    }
};

struct ID3FileInfo {
    TagHeaderEx tag{};
    mutable size_t mpegSize = 0;
    size_t getMpegSize(size_t totFileSize) const noexcept {

        totalFileSize = totFileSize;
        mpegStartPosition = tag.totalSizeInBytes();
        mpegSize = totalFileSize - tag.totalSizeInBytes();
        if (tag.hasv1Tag()) {
            if (mpegSize > ID3V1_TAG_SIZE) {
                mpegSize -= ID3V1_TAG_SIZE;
                mpegEndPosition = totalFileSize - ID3V1_TAG_SIZE;
            }
        }
        return mpegSize;
    }
    mutable size_t totalFileSize = 0;
    mutable size_t mpegStartPosition = 0;
    mutable size_t mpegEndPosition = 0;
};

static inline verifyTagResult verifyTag(TagHeaderEx& h) {
    std::string_view sv(h.ID, 3);
    if (h.version[0] != 3) return verifyTagResult::BadVersion;
    if (sv != "ID3") return verifyTagResult::BadID;
    if (h.sizeIndicator == 0) return verifyTagResult::BadSizeIndicator;
    std::bitset<8> f(h.flags);
    if (f.test(7)) h.hasUnsynchronisation = true;
    if (f.test(6)) h.hasExtendedHeader = true;
    if (f.test(5)) h.hasExperimental = true;
    f = f << 3;
    if (!f.none())
        return verifyTagResult::BadReservedFlags; // flags that are reserved are
                                                  // set.
    return verifyTagResult::OK;
}

static inline uint32_t decodeSynchSafe(uint32_t size) {
    size = (size & 0x0000007F) | ((size & 0x00007F00) >> 1)
        | ((size & 0x007F0000) >> 2) | ((size & 0x7F000000) >> 3);
    return size;
}

static inline uint32_t getTagSize(uint32_t sz) {
    swapEndian(sz);
    sz = decodeSynchSafe(sz);
    return sz;
}

static inline TagHeaderEx parseHeader(
    my::IDataReader& dr, bool expect_good = true) {

    using std::cerr;
    using std::endl;
    TagHeaderEx ret = {0};
    const auto data = dr.read(10);
    if (data.size() == 10) {
        memcpy(&ret, data.data(), TAG_HEADER_SIZE);
        ret.validity = verifyTag(ret);
        if (!expect_good) {
            if (ret.validity == ID3v2::verifyTagResult::OK) {
                cerr << "Expected duff tag here, but it seems to be valid?"
                     << endl;
                exit(-1000);
            }
            return ret;
        }
        ret.dataSizeInBytes = getTagSize(ret.sizeIndicator);
    }

    auto sk = dr.seek(-ID3V1_TAG_SIZE, std::ios_base::end);
    auto pos = dr.getPos();
    const auto sz = dr.getSize();
    if (pos != sz - 128) {
        cerr << "Nope, file should be at end after reading last 128 bytes"
             << endl;
#ifdef UNIT_TESTING
        exit(-10000);
#endif
    }
    const auto v1read = dr.readInto((char*)&ret.v1Tag, ID3V1_TAG_SIZE);
    if (dr.getPos() != dr.getSize()) {
        cerr << "After reading v1 tag, we should be at the end of the file"
             << endl;
#ifdef UNIT_TESTING
        exit(-10000);
#endif
        return ret;
    }
    sk = dr.seek(ID3v2::TAG_HEADER_SIZE);
    if (sk != ID3v2::TAG_HEADER_SIZE) {
        cerr << "Expected seek position to be after the valid header" << endl;
// assert("Expected seek position to be after the valid header" == 0);
#ifdef UNIT_TESTING
        exit(-10000);
#endif
        return ret;
    }

    return ret;
}

} // namespace ID3v2
