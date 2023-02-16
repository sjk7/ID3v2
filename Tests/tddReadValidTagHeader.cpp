#include "ID3v2.hpp"
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

int read_valid_header(){
    const auto fileName = utils::find_file_up("sample.mp3", "ID3");
    assert(!fileName.empty() );
    cout << "Mp3 file located at: " << fileName << endl;
    my::FileDataReader fdr(fileName);
    auto tag = ID3v2::parseHeader(fdr);
    assert(tag.validity == ID3v2::OK);
    assert(tag.dataSizeInBytes == 348);
    assert(tag.totalSizeInBytes() == tag.dataSizeInBytes + sizeof(ID3v2::TagHeader));

    return 0;
}

int main(){
    int ret =  read_valid_header();
    if (ret == 0) puts("OK");
    return 0;
}