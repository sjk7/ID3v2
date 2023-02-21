// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

#pragma once
#include "FileDataReader.hpp"
#include "myUtils.hpp"
#include <bitset>
#include <cstring> // memcpy
#include <unordered_map>
#include "ID3v1Genres.hpp"

namespace ID3v2 {

using uchar = unsigned char;
template <size_t SZ> using bytearray = std::array<uchar, SZ>;

static inline uint32_t swapEndian(std::uint32_t num) {
    uint32_t swapped = ((num >> 24) & 0xff) | // move byte 3 to byte 0
        ((num << 8) & 0xff0000) | // move byte 1 to byte 2
        ((num >> 8) & 0xff00) | // move byte 2 to byte 1
        ((num << 24) & 0xff000000); // byte 0 to byte 3
    return swapped;
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
// clang-format on

// we use a *signed* type here, because it may be used for seeking backward
static constexpr inline std::streamsize TAG_HEADER_SIZE = sizeof(TagHeader);

static_assert(std::is_pod_v<TagHeader>);

struct TagExtendedHeader {
    uint32_t sizeBytesIndicator; // $xx xx xx xx
    unsigned char flags[2]; // $xx xx
    uint32_t paddingSizeIndicator; // $xx xx xx xx
};

// we use a *signed* type here, because it may be used for seeking backward
static constexpr inline std::streamsize EXT_TAG_HEADER_SIZE
    = sizeof(TagExtendedHeader);
static_assert(std::is_pod_v<TagExtendedHeader>);

struct FrameHeader {
    // A tag must contain at least one frame.
    // A frame must be at least 1 byte big, excluding the header.
    char frameID[4]; // $xx xx xx xx (four characters)
    uint32_t sizeIndicator; // $xx xx xx xx
    std::byte Flags[2]; // $xx xx
};
// clang-format on
static_assert(std::is_pod_v<FrameHeader>);
// we use a *signed* type here, because it may be used for seeking backward
static constexpr inline std::streamsize FRAME_HEADER_SIZE
    = sizeof(TagExtendedHeader);

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
// we use a *signed* type here, because it may be used for seeking backward
static constexpr std::streamsize ID3V1_TAG_SIZE = 128;
#pragma pack(pop)

namespace detail {
    static bool IsFrameHeaderIDValid(const FrameHeader& f) {

        size_t i = 0;
        while (i < sizeof(f.frameID)) {
            const char c = f.frameID[i++];
            // std::cout << (c);
            if (i > 2) {
                if (!std::isalnum(static_cast<unsigned char>(c))) {
                    return false;
                }
            } else {

                if (!std::isalpha(static_cast<unsigned char>(c))) {
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
template <typename T, size_t size>
std::string fromFixed(const T (&array)[size]) {
    std::string ret = std::string(array, size);
    const auto found = ret.find('\0');
    if (found == std::string::npos) {
        return utils::strings::trim_copy(ret);
    }
    const auto val = ret.substr(0, found);
    return utils::strings::trim_copy(val);
}

enum class TagVersion { v1, v2 };
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
        assert(v1Checked != -1);
        std::string_view s{v1Tag.tag, 3};
        return s == "TAG";
    }

    std::string v1Artist() const noexcept { return fromFixed(v1Tag.artist); }
    std::string v1Title() const noexcept { return fromFixed(v1Tag.title); }

    std::string v1Year() const noexcept { return fromFixed(v1Tag.year); }
    std::string v1Album() const noexcept { return fromFixed(v1Tag.album); }
    std::string v1Comment() const noexcept { return fromFixed(v1Tag.comment); }
<<<<<<< HEAD
    unsigned char v1Genre() const noexcept { return v1Tag.genre; }
=======
    std::string v1Genre() const noexcept {
        const auto sz = ID3v1::GENRE_SIZE;
        const auto n = v1Tag.genre;
        if (n >= sz) return std::string{};
        return ID3v1::genres[n];
    }
>>>>>>> 896f57f5e77d74530500f07e152a8fbbf1366a24
    int v1year() const noexcept { return std::atoi(v1Year().c_str()); }

    int v1Checked = -1;
};

struct ID3FileInfo {

    const TagHeaderEx& Tag() const noexcept { return m_tag; }

    void SetTag(const TagHeaderEx& t) {
        m_tag = t;
        setPositions();
    }

    auto getMpegSize() const noexcept {

        if (m_tag.validity == verifyTagResult::OK) {
            assert(mpegSize);
        }
        return mpegSize;
    }

    auto TotalFileSize() const noexcept {
        assert(totalFileSize > 0);
        return totalFileSize;
    }
    auto MPEGStartPosition() const noexcept {
        assert(mpegStartPosition > 0);
        return mpegStartPosition;
    }

    auto MPEGEndPosition() const noexcept {
        assert(mpegEndPosition > 0);
        return mpegEndPosition;
    }
    const auto& FilePath() const noexcept { return filePath; }
    auto MPEGSize() const noexcept {
        assert(mpegSize > 0);
        return mpegSize;
    }

    // might throw if it can't get filesize
    void FilePathSet(const std::string& path) {
        filePath = path;
        fs::path p{path};
        totalFileSize = fs::file_size(p);
    }

    // this is data we found in the tag, but was totally undreadable to us.
    // You, as an educated caller, might know what to do with it
    std::string paddingJunk;

    private:
    mutable std::streamsize totalFileSize = -1;
    mutable std::streamoff mpegStartPosition = -1;
    mutable std::streamoff mpegEndPosition = -1;
    mutable std::streamsize mpegSize = -1;
    TagHeaderEx m_tag = {};
    std::string filePath;

    void setPositions() {
        assert(totalFileSize > 0); // set on filePathSet
        mpegStartPosition = m_tag.totalSizeInBytes();
        mpegSize = totalFileSize - m_tag.totalSizeInBytes();
        if (m_tag.hasv1Tag()) {
            if (mpegSize > ID3V1_TAG_SIZE) {
                mpegSize -= ID3V1_TAG_SIZE;
                mpegEndPosition = totalFileSize - ID3V1_TAG_SIZE;
            }
        } else {
            mpegEndPosition = totalFileSize;
        }
    }
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

    auto SizeInBytes(bool includingHeader = false) {
        if (sizeInBytes == 0) {
            sizeInBytes = GetFrameSize((FrameHeader) * this);
        }
        assert(m_info.TotalFileSize());
        if (sizeInBytes > m_info.TotalFileSize()) {
            throw std::runtime_error(
                m_info.FilePath() + "Frame size is more than the entire file!");
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
                    m_info.FilePath() + " cannot seek to data start position");
            }
        }

        const auto sz = SizeInBytes();
        allData.resize((size_t)(sz));
        const auto rd
            = static_cast<size_t>(dr.readInto(allData.data(), (size_t)sz));
        if (rd != sz) {
            throw std::runtime_error(m_info.FilePath()
                + " failed to read all the data from the file");
        }
        return allData.size();
    }

    // non syncsafe in 2.3, synchsafe in 2.4
    static inline size_t GetFrameSize(
        const FrameHeader& hdr, bool syncsafe = false) {
        auto sz = hdr.sizeIndicator;
        sz = swapEndian(sz);
        if (syncsafe) {
            const auto ret = decodeSynchSafe(sz);
            return ret;
        }
        return sz;
    }

    const std::string& AllData() const noexcept { return allData; }

    private:
    mutable std::streamsize sizeInBytes = 0;
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

static inline uint32_t getTagSize(uint32_t szInd) {
    auto sz = swapEndian(
        szInd); // v2.3: for the tag, it's sync safe (probably!) lol
    auto ret = decodeSynchSafe(sz);
    return ret;
}

static inline TagHeaderEx parseHeader(
    my::IDataReader& dr, const std::string& filePath) {

    using std::cerr;
    using std::endl;
    TagHeaderEx ret = {};
    const auto data = dr.read(10);
    if (data.size() == 10) {
        auto ptr = (TagHeader*)&ret;
        memcpy(ptr, data.data(), TAG_HEADER_SIZE);
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
    ret.v1Checked = 1;
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
        m_info.FilePathSet(filePath);
        m_info.SetTag(ID3v2::parseHeader(m_dr, filePath));
        const auto seekEndTag = m_info.Tag().totalSizeInBytes();

        const auto seeked = m_dr.seek(seekEndTag);
        (void)seeked;
        // Even for files that aren't even mp3 files,
        // I expect this to be OK: (coz noTag == 0 pos)
        assert(seeked == seekEndTag);
        f(m_dr, m_info);
    }

    ID3FileInfo m_info = {};

    std::string m_filePath;
    my::FileDataReader m_dr;
};

// populates the data in all the Tags; does not parse further.
// Calls CB() with each frame.
template <typename CB>
static inline void FillTags(ID3FileInfo& info, my::IDataReader& dr, CB&& cb) {

    using std::cerr;
    using std::endl;

    const auto& tag = info.Tag();
    const std::streamoff dataStartPos = tag.hasExtendedHeader
        ? sizeof(TagHeader) + sizeof(TagExtendedHeader)
        : sizeof(TagHeader);

    auto curPos = dr.getPos();
    if (dr.getPos() != dataStartPos) {
        const auto pos = dr.seek(dataStartPos);
        if (pos != dataStartPos) {
            throw std::runtime_error(
                info.FilePath() + ": could not seek to data start position");
        }
        curPos = pos;
    }

    const auto endPos = tag.dataSizeInBytes;

    while (dr.getPos() < endPos) {
        const auto pos = dr.getPos();
        FrameHeaderEx hdr(info, pos);
        dr.readInto(&hdr, sizeof(FrameHeader));
        if (IsFrameHeaderValid(hdr)) {
            const auto frameSize = hdr.ReadData(dr);
            assert(frameSize > 0);
            (void)frameSize;
            cb(hdr);
        } else {
            // we read 10 bytes when attempting to read a head
            dr.seek(-ID3v2::TAG_HEADER_SIZE, std::ios::cur);
#ifndef NDEBUG
            const auto whereNow = dr.getPos();
            assert(whereNow == pos);
#endif
            const auto mpegStart = info.MPEGStartPosition();
            const auto remain_tag = mpegStart - pos;
            if (remain_tag > 0) {

                std::string& padding = info.paddingJunk;
                dr.readInto(padding, (size_t)remain_tag);
                const auto justPaddingRemains = std::all_of(
                    padding.begin(), padding.end(), [](const char c) {
                        if (c == '\0') return true;
                        return false;
                    });
                if (justPaddingRemains) {
                    return;
                } else {
                    const auto junkPos = padding.find_first_not_of('\0');
                    const auto wtf = padding.substr(junkPos);
                    std::cout << wtf << std::endl;
                    std::cout << junkPos << std::endl;
                    MYTRACE(MyTraceFlags::defaults, info.FilePath(),
                        "Has appparent junk after the tags, and before the "
                        "start of audio. info.paddingJunk contains the junk");
                    return;
                }
            }
            MYTRACE(MyTraceFlags::defaults, info.FilePath(),
                "Have invalid ID3v2 Frame Header at file position: ", pos);
        }
    }
}

// This class will spit out any valid ID3v2frames in CB.
// It spits out the whole of the data, comprised of the header,
// followed by all the data in the frame.
struct TagDataReader {
    // clang-format off
    template <typename CB> TagDataReader
    (const std::string &filePath, CB &&cb) : m_dr(filePath)
    // clang-format on
    {

        m_info.FilePathSet(filePath);
        assert(m_dr.is_open());
        m_info.SetTag(ID3v2::parseHeader(m_dr, filePath));

        if (m_info.Tag().validity != ID3v2::verifyTagResult::OK) {
            throw std::runtime_error("File: " + filePath + " has no ID3v2Tags");
        }

        ID3v2::FillTags(m_info, m_dr, std::forward<CB>(cb));
    }

    my::FileDataReader m_dr;
    ID3v2::ID3FileInfo m_info = {};
};

enum class TextEncodingType { ansi, unicode_with_bom };

struct AbstractFrame {
    AbstractFrame(const FrameHeaderEx& h) : m_hdr(h) {
        const auto& raw = h.AllData();
        if (raw.empty()) throw std::runtime_error("Abstract Frame: no data");
    }
    virtual ~AbstractFrame() = default;
    FrameHeader hdr{};
    std::string data;
    std::string& allData() noexcept { return data; }
    FrameHeaderEx m_hdr;
    TextEncodingType textEncoding = TextEncodingType::ansi;
    // regardless of the type of tag, this is where we store the actual data
    std::string payLoad;
    const std::string& PayLoad() const noexcept { return payLoad; }
    std::string m_suid;
    virtual const std::string& uid() const noexcept { return m_suid; }
    bool malFormed = false;
};
using ptrBase = std::unique_ptr<AbstractFrame>;

struct TextFrame : AbstractFrame {
    TextFrame(const FrameHeaderEx& h) : AbstractFrame(h) {
        // clang-format off
        /*/
        <Header for 'Text information frame', ID: "T000" - "TZZZ", excluding "TXXX" 
        Text encoding    $xx Information    <text string according to
        encoding>
        /*/
        // clang-format on
        textEncoding = static_cast<TextEncodingType>(*h.AllData().begin());
        payLoad = h.AllData().substr(1, h.AllData().size() - 1);
        this->m_suid = h.IDString();
    }
    virtual ~TextFrame() = default;
};
struct CommentsFrame : AbstractFrame {
    CommentsFrame(const FrameHeaderEx& h) : AbstractFrame(h) {
        // clang-format off
        /*/
        <Header for 'Comment', ID: "COMM">
        0            Text encoding           $xx
        123          Language                $xx xx xx
        4->found     Short content descrip.  <text string according to encoding> $00 (00)
        found+1->end The actual text         <full text string according to encoding>
        /*/
        // clang-format on
        const auto& s = h.AllData();
        this->m_suid = h.IDString();
        this->textEncoding = static_cast<TextEncodingType>(*s.begin());

        const auto nNulls
            = this->textEncoding == TextEncodingType::ansi ? 1 : 2;
        const std::string nullstr(nNulls, '\0');
        const auto found = s.find(nullstr, 4);
        if (found == std::string::npos) {
            // DPS: no lang, no desc, just a null then some string, with no
            // nulls in it
            this->payLoad = s.substr(1);
            if (std::all_of(this->payLoad.begin(), this->payLoad.end(),
                    [](const char c) { return c != 0; })) {
                this->malFormed = true;
                return; // good enuf
            } else {

                this->payLoad.clear();
                throw std::runtime_error(
                    "CommentsFrame: no null found to indicate where the "
                    "description ends and the text starts");
            }
        }

        m_lang = s.substr(1, 3);
        m_description = s.substr(4, found);
        this->payLoad = s.substr(found + nNulls);
    }
    virtual ~CommentsFrame() = default;
    std::string m_description;
    std::string m_lang;
    const std::string& contentDescription() const noexcept {
        return m_description;
    }
    const std::string& language() const noexcept { return m_lang; }
};

struct UserTextFrame : AbstractFrame {

    UserTextFrame(const FrameHeaderEx& h) : AbstractFrame(h) {
        // clang-format off
        /*/
            <Header for 'User defined text information frame', ID: "TXXX">
            0                       Text encoding    $xx
            1->found                Description    <text string according to encoding> $00 (00)
            found+nullSize, remain  Value    <text string according to encoding>
        /*/
        // clang-format on
        const auto& s = h.AllData();
        this->textEncoding = static_cast<TextEncodingType>(*s.begin());
        const auto nNulls
            = this->textEncoding == TextEncodingType::ansi ? 1 : 2;
        const std::string nullstr(nNulls, '\0');
        const auto found_null = s.find(nullstr, 1);

        if (found_null == std::string::npos)
            throw std::runtime_error("UserTextFrame: could not find separating "
                                     "null between description and value");
        this->m_description = s.substr(1, found_null - 1);
        this->payLoad = s.substr(found_null + 1);
        this->m_suid = "TXXX:" + m_description;
    }
    virtual ~UserTextFrame() = default;

    std::string m_description;
    const std::string& contentDescription() const noexcept {
        return m_description;
    }
};

struct PictureFrame : AbstractFrame {

    PictureFrame(const FrameHeaderEx& h) : AbstractFrame(h) {
        // clang-format off
       /*/
       <Header for 'Attached picture', ID: "APIC">
       0     Text encoding   $xx
            MIME type       <text string> $00
            Picture type    $xx
            Description     <text string according to encoding> $00 (00)
            Picture data    <binary data>
        /*/
        // clang-format on
        const auto& s = h.AllData();
        this->textEncoding = static_cast<TextEncodingType>(*s.begin());
        int nNulls = 1; // encoding doesn't apply to mimetype

        std::string nullstr(nNulls, '\0');
        auto found_null = s.find(nullstr, 1);
        if (found_null == std::string::npos) {
            throw std::runtime_error("PictureFrame: cannot find null separator "
                                     "between mime-type and picture-type.");
        }
        mimeType = s.substr(1, found_null);
        if (s.size() < found_null + 1)
            throw std::runtime_error("PictureFrame: not enough data");
        pictureType = *(s.data() + found_null + 1);

        nNulls = this->textEncoding == TextEncodingType::ansi ? 1 : 2;

        nullstr = std::string(nNulls, '\0');
        auto pos = found_null + 1;

        found_null = s.find(nullstr, pos);
        if (found_null == std::string::npos) {
            throw std::runtime_error("PictureFrame: cannot find null separator "
                                     "between description and data");
        }
        ++pos; // we found a null
        const int len_desc = (int)(found_null - (pos)); // may be zero
        assert(len_desc >= 0);
        this->description = s.substr(pos, len_desc); // can be zero length
        pos += description.size() + nNulls; // there is a null after description
        this->payLoad = s.substr(pos);
        this->m_suid = "APIC:" + std::to_string(pictureType);
        // std::fstream f("ffs.png", std::ios::out | std::ios::binary);
        // f.write(this->payLoad.data(), this->payLoad.size());
        // f.close();
    }
    virtual ~PictureFrame() = default;
    int pictureType = 0;
    std::string description;
    std::string mimeType;
};

struct TagFactory {
    static inline ptrBase MakeTextFrame(const FrameHeaderEx& h) {
        ptrBase ret = std::make_unique<TextFrame>(h);
        return ret;
    }

    static inline ptrBase MakeCommentsFrame(const FrameHeaderEx& h) {
        ptrBase ret = std::make_unique<CommentsFrame>(h);
        return ret;
    }

    static inline ptrBase MakeUserTextFrame(const FrameHeaderEx& h) {
        ptrBase ret = std::make_unique<UserTextFrame>(h);
        return ret;
    }

    static inline ptrBase MakePictureFrame(const FrameHeaderEx& h) {
        ptrBase ret = std::make_unique<PictureFrame>(h);
        return ret;
    }
};

struct TagCollection {

    using ContainerType = std::unordered_map<std::string, ptrBase>;

    private:
    ContainerType m_tags;
    // TagFactory Factory;
    TagCollection() = default;

    public:
    TagCollection(const std::string& filepath) : TagCollection() {
        TagDataReader rdr(filepath, [&](const FrameHeaderEx& h) {
            // bool handled = false;

            if (h.frameID[0] == 'T') {
                // Text or Comments Frame
                if (h.IDString() == "TXXX") {
                    auto ptr = TagFactory::MakeUserTextFrame(h);
                    if (ptr) m_tags.insert({ptr->uid(), std::move(ptr)});
                    // handled = true;
                } else {
                    auto ptr = TagFactory::MakeTextFrame(h);
                    if (ptr) m_tags.insert({ptr->uid(), std::move(ptr)});
                    // handled = true;
                }
            } else {

                if (h.IDString() == "COMM") {
                    auto ptr = TagFactory::MakeCommentsFrame(h);
                    if (ptr) m_tags.insert({ptr->uid(), std::move(ptr)});
                    // handled = true;
                } else {
                    if (h.IDString() == "APIC") {
                        auto ptr = TagFactory::MakePictureFrame(h);
                        if (ptr) m_tags.insert({ptr->uid(), std::move(ptr)});
                        // handled = true;
                    }
                }
            }
        });
        info = rdr.m_info;
    }
    ~TagCollection() {}

    const ContainerType& Tags() const noexcept { return m_tags; }
    ID3v2::ID3FileInfo info = {};

    const std::string& tagFromID(const std::string& id) const noexcept {
        const auto& f = m_tags.find(id);
        if (f == m_tags.end()) return m_empty;
        return f->second->PayLoad();
    }
    const std::string& Artist() const noexcept { return tagFromID("TPE1"); }
    const std::string& Title() const noexcept { return tagFromID("TIT2"); }
    const std::string& Album() const noexcept { return tagFromID("TALB"); }
    const std::string& UserTag(const std::string id) const noexcept {
        return tagFromID("TXXX:" + id);
    }
    const std::string& Comment() const noexcept { return tagFromID("TCOMM"); }

    private:
    static inline std::string m_empty;
};

} // namespace ID3v2
