#include "../include/ID3v2.hpp"
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

int test_id3v2_headers() {
    ID3v2::TagHeaderEx th;
    int verif = ID3v2::verifyTag(th);
    if (verif != -1) {
        cerr << "No. Expected -1 from verif" << endl;
        return -1;
    }

    th.version[0] = 3;
    verif = ID3v2::verifyTag(th);
    if (verif != -2) {
        cerr << "No. Expected -2 from verif" << endl;
        return -1;
    }

    memcpy(th.ID, "ID3", 3);
    verif = ID3v2::verifyTag(th);
    if (verif != -4) {
        cerr << "No. Expected -4 from verif" << endl;
        return -1;
    }

    const auto fn = utils::find_file_up("testfile.txt", "ID3");
    if (fn.empty()) {
        assert("testfile.txt cannot be found, cannot continue" == nullptr);
        return -1;
    }
    my::FileDataReader myReader(fn);
    ID3v2::TagHeaderEx h = ID3v2::parseTag(myReader);
    if (h.validity == ID3v2::OK) {
        cerr << "No way should this tag be valid!" << endl;
        return -1;
    }
    assert(h.validity != ID3v2::OK);

    return 0;
}

int main() {
    int ret = test_id3v2_headers();
    return ret;
}
