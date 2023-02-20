// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

#define CATCH_CONFIG_MAIN
#include "catch2KLJ.hpp"
#include "ID3v2.hpp"
#include <iostream>
using namespace std;

TEST_CASE("Test ID3Skipper on known bad file") {
    const auto filePath = utils::find_file_up("testfile.txt", "ID3");
    REQUIRE_MESSAGE("testfile.txt needs to be available.", !filePath.empty());

    ID3v2::ID3FileInfo myInfo = {};
    bool threw = false;

    try {

        ID3v2::ID3v2Skipper skipper(filePath.string(),
            [&myInfo](my::FileDataReader&, ID3v2::ID3FileInfo& info) {
                cout << "Header size is:" << info.Tag().dataSizeInBytes << endl;
                myInfo = info;
            });
    } catch (const std::exception&) {
        threw = true;
    }
    REQUIRE_MESSAGE("Should have thrown on bad file", threw);
    REQUIRE_MESSAGE("Unexpected tag validity result",
        (myInfo.Tag().validity == ID3v2::verifyTagResult::BadVersion));
}

TEST_CASE("Test ID3Skipper on known good file") {

    using std::cout;
    const auto filePath = utils::find_file_up("sample.mp3", "ID3v2");
    REQUIRE_MESSAGE("Must be able to find file sample.mp3", !filePath.empty());

    ID3v2::ID3FileInfo myInfo = {};

    ID3v2::ID3v2Skipper skipper(filePath.string(),
        [&myInfo](my::FileDataReader&, ID3v2::ID3FileInfo& info) {
            cout << "Header size is:" << info.Tag().dataSizeInBytes << endl;
            myInfo = info;
        });

    auto& dr = skipper.m_dr;
    const auto pos = dr.getPos();
    cout << "Audio starts at position: " << pos << endl;
    REQUIRE(pos == myInfo.MPEGStartPosition());
    std::string mpegData;
    auto got = dr.readInto(mpegData, myInfo.MPEGSize());
    REQUIRE(got == myInfo.MPEGSize());

    // const auto szPrint = std::min(mpegData.size(), size_t(150));
    //  cout << "Here's some mpeg data (could be padding thou) ... \n\n"
    //       << mpegData.substr(0, szPrint) << std::endl;
    unsigned char c1 = mpegData[0];
    unsigned char c2 = mpegData[1];
    cout << "First two bytes are " << (int)c1 << " " << (int)c2 << endl;
    REQUIRE(c1 == 255);
    REQUIRE(c2 >= 251);
}