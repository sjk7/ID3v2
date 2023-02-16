#include "../include/FileDataReader.hpp"
#include "../include/myUtils.hpp"
#include <array>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstddef> // std::byte
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using namespace utils;

namespace ID3v2
{

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
};

static inline int verifyTag(TagHeaderEx &h)
{
    std::string_view sv(h.ID, 3);
    if (h.version[0] != 3)
        return -1;
    if (sv != "ID3")
        return -2;
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
static inline TagHeaderEx parseTag(IDataReader &dr)
{
    TagHeaderEx ret = {0};
    const auto data = dr.read(10);
    memcpy(&ret, data.data(), TAG_HEADER_SIZE);
    ret.validity = verifyTag(ret);
    return ret;
}

} // namespace ID3v2

using namespace std;
int tdd_file();

int test_file_reader()
{
    const auto filepath = utils::find_file_up("testfile.txt", "ID3v2");
    assert(!filepath.empty());
    try
    {
        FileDataReader reader(filepath);
        cout << reader.getSize();
        if (reader.getSize() != 27)
        {
            std::cerr << "Unexpected. File size is 27" << endl;
        }
        std::string mybuf;
        auto bytesRead = reader.readInto(mybuf, 10);
        if (bytesRead != 10)
        {
            cerr << "BytesRead should be 10";
            return -10;
        }

        if (mybuf.size() != 10)
        {
            cerr << "mybuf.size() shoule be 10";
            return -10;
        }
        if (mybuf != "ABCDEFGHIJ")
        {
            cerr << "my buf should have correct contents.";
            return -11;
        }
        if (reader.getPos() != 10)
        {
            cerr << "my buf position should be 10";
            return -12;
        }

        auto nowRead = reader.read(5);
        if (reader.getPos() != 15)
        {
            cerr << "reader position, expected 15";
            return -15;
        }

        if (reader.data().size() != 5)
        {
            cerr << "There should be a 5 byte buffer.";
            return -15;
        }

        // now, this should throw, but only once all the
        // data is exhausted.
        bool thrown = false;
        try{
            auto blowed_up = reader.read(1000);
            // cout << "actual bytes read = " << blowed_up << endl;
            blowed_up= reader.read(1);
        }catch(const std::exception& e){
            // cout << "Correctly caught " << e.what() << endl;
            thrown = true;
            auto posNow = reader.getPos();
            if (posNow != 27){
                cerr << "Unexpected: file pos ought to be 27 hr";
                return -88;
            }

            
        }

        if (!thrown){
            cerr << "Nope, it should have thrown with a crazy seek";
        }
        bool ok = false;
        auto newpos = reader.seek(0, ok);
        if (!ok){
            cerr << "unexpected: seek to 0 should work";
            return -1;
        }
        if (newpos != 0){
            cerr << "newpos should be 0";
            return -1;
        }
        auto posback = reader.getPos();
        if (posback != 0){
            cerr << "posback ought to be 0";
            return -1;
        }

        // seeking off the end behaviour
        auto seeked = reader.seek(10000, ok);
        if (!ok){
            cerr << "As mad as it is, seeking beyond the end of a file is ok!";
            return -1;
        }
        assert(seeked == 10000);

        // check seeking backwards from the end
        seeked = reader.seek(0, ok, std::ios_base::seekdir::end);
        assert(seeked == 27);
        seeked = reader.seek(-2, ok, std::ios_base::seekdir::end);
        assert(seeked = 25);
        auto two = reader.read(2);
        assert(two.size() == 2);

        std::string sbak;
        auto got = reader.readInto(sbak, 10);
        assert(got == 0); //coz eof, and no exception here
    }
    catch (const std::exception &e)
    {
        cerr << "UNEXPECTED failure opening file" << endl;
        return -1;
    }

    return 0;
}

int main(int, char **argv)
{

    cout << "Project running in: " << argv[0] << endl;
    int ret = test_file_reader();
    return ret;
}
