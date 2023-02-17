// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

#define UNIT_TESTING 77
#include "../include/ID3v2.hpp"
#include <iostream>

#ifdef _MSC_VER
#pragma warning(                                                               \
    disable : 4130) // logical operation on address of string constant bullshit.
                    // (assert("ffs" == nullptr))
#endif

using std::cerr;
using std::cout;
using std::endl;

int test_invalid_id3v2_headers() {
    ID3v2::TagHeaderEx th{};
    auto verif = ID3v2::verifyTag(th);
    if (verif != ID3v2::verifyTagResult::BadVersion) {
        cerr << "No. Expected invalid from verif" << endl;
        return -1;
    }

    th.version[0] = 3;
    verif = ID3v2::verifyTag(th);
    if (verif != ID3v2::verifyTagResult::BadID) {
        cerr << "No. Expected BadID from verif" << endl;
        return -1;
    }

    memcpy(th.ID, "ID3", 3);
    verif = ID3v2::verifyTag(th);
    if (verif != ID3v2::verifyTagResult::BadSizeIndicator) {
        cerr << "No. Expected BadSizeIndicator from verifytag" << endl;
        return -1;
    }

    const auto fn = utils::find_file_up("testfile.txt", "ID3");
    if (fn.empty()) {
        assert("testfile.txt cannot be found, cannot continue" == nullptr);
        return -1;
    }
    my::FileDataReader myReader(fn);
    ID3v2::TagHeaderEx h = ID3v2::parseHeader(myReader, "Bad Tag in memory");
    if (h.validity == ID3v2::verifyTagResult::OK) {
        cerr << "No way should this tag be valid!" << endl;
        return -1;
    }
    // assert(h.validity != ID3v2::OK);

    return 0;
}

int main() {
    int ret = test_invalid_id3v2_headers();
    return ret;
}
