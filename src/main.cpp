// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

#include "ID3v2.hpp"
#include <iostream>
using namespace std;

// Convenience class for when you are not interested
// in the actual tags, but you want a reader opened beyond
// the id3v2Tag (say, if you are an audio parser), and
// to populate where the audio ends, as well as where it starts.
// V1 tags are taken  into consideration here.
struct ID3v2Skipper {

    template <typename F>
    ID3v2Skipper(const std::string& filePath, F&& f) : m_dr(filePath) {
        m_info.tag = ID3v2::parseHeader(m_dr);
        const auto fsize = m_dr.getSize();
        m_info.getMpegSize(fsize);
        auto seeked = m_dr.seek(m_info.tag.totalSizeInBytes());
        assert(seeked = m_info.tag.totalSizeInBytes());
        f(m_dr, m_info);
    }

    ID3v2::ID3FileInfo m_info = {};

    std::string m_filePath;
    my::FileDataReader m_dr;
};

int main() {

    using std::cout;
    const auto filePath = utils::find_file_up("sample.mp3", "ID3");
    assert(!filePath.empty());

    ID3v2::ID3FileInfo myInfo = {0};

    ID3v2Skipper skipper(
        filePath, [&myInfo](my::FileDataReader& rdr, ID3v2::ID3FileInfo& info) {
            cout << "Header size is:" << info.tag.dataSizeInBytes << endl;
            myInfo = info;
        });

    auto& dr = skipper.m_dr;
    const auto pos = dr.getPos();
    cout << "Audio starts at position: " << pos << endl;
    assert(pos == myInfo.mpegStartPosition);
    std::string mpegData;
    auto got = dr.readInto(mpegData, myInfo.mpegSize);
    assert(got == myInfo.mpegSize);
    if (got != myInfo.mpegSize) {
        cerr << "Trouble reading all of the mpeg data" << endl;
        return -1;
    }
    const auto szPrint = std::min(mpegData.size(), size_t(150));
    cout << "Here's some mpeg data (could be padding thou) ... \n\n"
         << mpegData.substr(0, szPrint) << std::endl;
    unsigned char c1 = mpegData[0];
    unsigned char c2 = mpegData[1];
    cout << "First two bytes are " << (int)c1 << " " << (int)c2 << endl;
}