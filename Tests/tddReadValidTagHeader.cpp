// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

#include "ID3v2.hpp"
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

int read_valid_header() {
    using std::cerr;
    using std::endl;

    const auto fileName = utils::find_file_up("sample.mp3", "ID3");
    assert(!fileName.empty());
    cout << "Mp3 file located at: " << fileName << endl;
    my::FileDataReader fdr(fileName);
    auto tag = ID3v2::parseHeader(fdr);
    if (tag.validity != ID3v2::verifyTagResult::OK) {
        cerr << "No. Expected a valid tag here" << endl;
        return -90;
    }
    if (tag.dataSizeInBytes != 348) {
        cerr << "No. Expected a valid tag here, with size == 348" << endl;
        return -91;
    }
    if (tag.totalSizeInBytes()
        != tag.dataSizeInBytes + sizeof(ID3v2::TagHeader)) {
        cerr << "No. Tag sizes not sane." << endl;
        return -92;
    }

    return 0;
}

int main() {
    int ret = read_valid_header();
    if (ret == 0) puts("OK");
    return 0;
}