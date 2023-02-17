// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com
#define CATCH_CONFIG_MAIN
#include "ID3v2.hpp"
#include "catch2.hpp"
#include <iostream>
using namespace std;

TEST_CASE("Test ID3Skipper on known bad file")
{
    const auto filePath = utils::find_file_up("testfile.txt", "ID3");
    REQUIRE(!filePath.empty());

    ID3v2::ID3FileInfo myInfo = {0};

    ID3v2::ID3v2Skipper skipper(filePath, [&myInfo](my::FileDataReader &rdr, ID3v2::ID3FileInfo &info) {
        cout << "Header size is:" << info.tag.dataSizeInBytes << endl;
        myInfo = info;
    });

    REQUIRE(myInfo.tag.validity == ID3v2::verifyTagResult::BadVersion);
}

TEST_CASE("Test ID3Skipper on known good file")
{

    using std::cout;
    const auto filePath = utils::find_file_up("sample.mp3", "ID3");
    REQUIRE(!filePath.empty());

    ID3v2::ID3FileInfo myInfo = {0};

    ID3v2::ID3v2Skipper skipper(filePath, [&myInfo](my::FileDataReader &rdr, ID3v2::ID3FileInfo &info) {
        cout << "Header size is:" << info.tag.dataSizeInBytes << endl;
        myInfo = info;
    });

    auto &dr = skipper.m_dr;
    const auto pos = dr.getPos();
    cout << "Audio starts at position: " << pos << endl;
    REQUIRE(pos == myInfo.mpegStartPosition);
    std::string mpegData;
    auto got = dr.readInto(mpegData, myInfo.mpegSize);
    REQUIRE(got == myInfo.mpegSize);

    const auto szPrint = std::min(mpegData.size(), size_t(150));
    cout << "Here's some mpeg data (could be padding thou) ... \n\n" << mpegData.substr(0, szPrint) << std::endl;
    unsigned char c1 = mpegData[0];
    unsigned char c2 = mpegData[1];
    cout << "First two bytes are " << (int)c1 << " " << (int)c2 << endl;
}