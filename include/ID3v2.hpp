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
static inline uint32_t decodeSynchSafe(uint32_t size) {
    size = (size & 0x0000007F) | ((size & 0x00007F00) >> 1)
        | ((size & 0x007F0000) >> 2) | ((size & 0x7F000000) >> 3);
    return size;
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
    char frameID [4];       // $xx xx xx xx (four characters)
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

struct AbstractFrame {
    FrameHeader hdr;
    std::string data;
    std::string& allData() noexcept { return data; }
};
using ptrBase = std::unique_ptr<AbstractFrame>;

struct TextFrame : AbstractFrame {};
struct CommentsFrame : TextFrame {};

namespace detail {
    static bool IsFrameHeaderIDValid(const FrameHeader& f) {

        size_t i = 0;
        while (i < sizeof(f.frameID)) {
            const char c = f.frameID[i++];
            // std::cout << (c);
            if (i > 2) {
                if (!std::isalnum(c)) {
                    return false;
                }
            } else {

                if (!std::isalpha(c)) {
                    return false;
                }
            }
        }
        return true;
    }

    /*/
    static inline ptrBase parseFrame(const FrameHeader& f) {
        std::string_view id{f.frameID, 3};
        if (f.frameID[0] == 'T') {
            return parseTextRelatedFrame(f);
        }
        return nullptr;
    }
    /*/

} // namespace detail

enum class verifyTagResult {
    BadBase = -1,
    OK = 1,
    BadVersion = BadBase - 1,
    BadID = BadBase - 2,
    BadSizeIndicator = BadBase - 3,
    BadReservedFlags = BadBase - 4
};

static inline std::string ValidityToString(
    const verifyTagResult& v, const TagHeader& h) {

    unsigned int a = static_cast<unsigned int>(v);
    unsigned int highest = static_cast<unsigned int>(verifyTagResult::OK);
    unsigned int lowest
        = static_cast<unsigned int>(verifyTagResult::BadReservedFlags);

    if (a > highest || a < lowest) {
        return "Invalid and out-of-range verifyTagResult";
    }
    switch (v) {
        case verifyTagResult::OK: return "No Verification Error";
        case verifyTagResult::BadBase: return "Unknown Verification Error";
        case verifyTagResult::BadVersion: return "Bad ID3v2(.3) version";
        case verifyTagResult::BadID:
            return "Bad ID: [" + std::string(h.ID, 3) + "]";
        case verifyTagResult::BadSizeIndicator: return "Bad Size Indicator";
        case verifyTagResult::BadReservedFlags: return "Bad reserved flags";
        default: return "Out of range verification error";
    }
}

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
    std::string filePath;
};

struct FrameHeaderEx : FrameHeader {

    private:
    const ID3FileInfo& m_info;
    const std::streamoff filePosition = 0;
    std::string allData;

    public:
    FrameHeaderEx(const ID3FileInfo& info, const std::streamoff& filePos)
        : m_info(info), filePosition(filePos) {}

    std::string_view IDString() const noexcept {
        return std::string_view((char*)this, 4);
    }
    size_t SizeInBytes(bool includingHeader = false) {
        if (sizeInBytes == 0) {
            sizeInBytes = GetFrameSize(*this);
        }
        assert(m_info.totalFileSize);
        if (sizeInBytes > m_info.totalFileSize) {
            throw std::runtime_error(
                m_info.filePath + "Frame size is more than the entire file!");
        }
        return includingHeader ? sizeInBytes + sizeof(FrameHeader)
                               : sizeInBytes;
    }

    size_t ReadData(my::IDataReader& dr) {
        const auto loc = std::streamoff(filePosition + sizeof(FrameHeader));
        if (dr.getPos() != loc) {
            const auto seeked = dr.seek(loc);
            if (seeked != loc) {
                throw std::runtime_error(
                    m_info.filePath + " cannot seek to data start position");
            }
        }

        const auto sz = SizeInBytes();
        allData.resize(sz);
        const auto rd = dr.readInto(allData.data(), sz);
        if (rd != sz) {
            throw std::runtime_error(
                m_info.filePath + " failed to read all the data from the file");
        }
        return allData.size();
    }

    static inline size_t GetFrameSize(const FrameHeader& hdr) {
        auto sz = hdr.sizeIndicator;
        swapEndian(sz);
        return decodeSynchSafe(sz);
    }

    const std::string& AllData() const noexcept { return allData; }

    private:
    mutable size_t sizeInBytes = 0;
};

static inline bool IsFrameHeaderValid(const FrameHeaderEx& f) {
    bool b = detail::IsFrameHeaderIDValid(f);
    return b;
}

static inline verifyTagResult verifyTag(TagHeaderEx& h) {

    // although it's faster not to look at the ID first,
    // we ought to check it first so as not to surprise callers.
    {
        std::string_view sv(h.ID, 3);
        if (sv != "ID3") return verifyTagResult::BadID;
    }
    if (h.sizeIndicator == 0) return verifyTagResult::BadSizeIndicator;
    if (h.version[0] != 3) return verifyTagResult::BadVersion;

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

static inline uint32_t getTagSize(uint32_t sz) {
    swapEndian(sz);
    sz = decodeSynchSafe(sz);
    return sz;
}

static inline TagHeaderEx parseHeader(
    my::IDataReader& dr, const std::string& filePath) {

    using std::cerr;
    using std::endl;
    TagHeaderEx ret = {};
    const auto data = dr.read(10);
    if (data.size() == 10) {
        memcpy(&ret, data.data(), TAG_HEADER_SIZE);
        ret.validity = verifyTag(ret);
        if (ret.validity == verifyTagResult::OK) {
            if (ret.sizeIndicator > 0)
                ret.dataSizeInBytes = getTagSize(ret.sizeIndicator);
        } else {
            throw std::runtime_error(filePath + "v2 Tag is not valid"
                + ValidityToString(ret.validity, ret));
        }
    }

    const auto sz = dr.getSize();
    if (sz > ID3V1_TAG_SIZE) {
        dr.seek(-ID3V1_TAG_SIZE, std::ios_base::end);
        auto pos = dr.getPos();
        if (pos != sz - 128) {
            cerr << "Nope, file should be at end after reading last 128 bytes"
                 << endl;
        }
        dr.readInto((char*)&ret.v1Tag, ID3V1_TAG_SIZE);
        if (dr.getPos() != dr.getSize()) {
            cerr << "After reading v1 tag, we should be at the end of the file"
                 << endl;
            return ret;
        }
    }
    if (ret.validity == verifyTagResult::OK) {
        const auto sk = dr.seek(ID3v2::TAG_HEADER_SIZE);
        if (sk != ID3v2::TAG_HEADER_SIZE) {
            cerr << "Expected seek position to be after the valid header"
                 << endl;
        }
    }

    return ret;
}

// Convenience class for when you are not interested
// in the actual tags, but you want a reader opened beyond
// the id3v2Tag (say, if you are an audio parser), and
// to populate where the audio ends, as well as where it starts.
// V1 tags are taken  into consideration here. You can re-use the
// provider FileReader, for efficiency.
auto empty_lambda = [](my::FileDataReader&, ID3v2::ID3FileInfo&) { return; };

struct ID3v2Skipper {
    template <typename F>
    ID3v2Skipper(const std::string& filePath, F&& f = empty_lambda)
        : m_dr(filePath) {
        m_info.filePath = filePath;
        m_info.tag = ID3v2::parseHeader(m_dr, filePath);
        const auto fsize = m_dr.getSize();
        m_info.getMpegSize(fsize);
        const auto seekEndTag = m_info.tag.totalSizeInBytes();
        const auto seeked = m_dr.seek(seekEndTag);
        // Even for files that aren't even mp3 files,
        // I expect this to be OK: (coz noTag == 0 pos)
        assert(seeked == seekEndTag);
        f(m_dr, m_info);
    }

    ID3FileInfo m_info = {};

    std::string m_filePath;
    my::FileDataReader m_dr;
};

template <typename CB>
static inline void FillTags(
    const ID3FileInfo& info, my::IDataReader& dr, CB&& cb) {

    const std::streamoff dataStartPos = info.tag.hasExtendedHeader
        ? sizeof(TagHeader) + sizeof(TagExtendedHeader)
        : sizeof(TagHeader);

    auto curPos = dr.getPos();
    if (dr.getPos() != dataStartPos) {
        const auto pos = dr.seek(dataStartPos);
        if (pos != dataStartPos) {
            throw std::runtime_error(
                info.filePath + ": could not seek to data start position");
        }
        curPos = pos;
    }

    const auto endPos = info.tag.dataSizeInBytes;

    while (dr.getPos() < endPos) {
        const auto pos = dr.getPos();
        FrameHeaderEx hdr(info, pos);
        dr.readInto(&hdr, sizeof(FrameHeader));
        if (IsFrameHeaderValid(hdr)) {

            const auto frameSize = hdr.ReadData(dr);
            assert(frameSize > 0);
            cb(hdr);
        } else {
            assert(0);
        }
    }
}

// This class will spit out any valid ID3v2frames in CB.
// It spits out the whole of the data, comprised of the header,
// followed by all the data in the frame.
struct TagDataReader {
    template <typename CB>
    TagDataReader(const std::string& filePath, CB&& cb) : m_dr(filePath) {
        m_info.filePath = filePath;
        m_info.totalFileSize = m_dr.getSize();
        m_info.tag = ID3v2::parseHeader(m_dr, filePath);
        if (m_info.tag.validity != ID3v2::verifyTagResult::OK) {
            throw std::runtime_error("File: " + filePath + " has no ID3v2Tags");
        }

        ID3v2::FillTags(m_info, m_dr, std::forward<CB>(cb));
    }

    my::FileDataReader m_dr;
    ID3v2::ID3FileInfo m_info = {};
};

} // namespace ID3v2
