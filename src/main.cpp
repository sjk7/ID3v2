// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

#include "catch2.hpp"
#include "ID3v2.hpp"
#include <iostream>
using namespace std;



int main() {

    using std::cout;
    const auto filePath = utils::find_file_up("sample.mp3", "ID3");
    assert(!filePath.empty());

    ID3v2::ID3FileInfo myInfo = {0};

    ID3v2::ID3v2Skipper skipper(
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