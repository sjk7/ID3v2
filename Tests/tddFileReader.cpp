// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

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

using namespace std;
using namespace my;
int tdd_file();

int test_file_reader() {
    const auto filepath = utils::find_file_up("testfile.txt", "ID3v2");
    assert(!filepath.empty());
    try {
        FileDataReader reader(filepath);
        // cout << reader.getSize();
        if (reader.getSize() != 27) {
            std::cerr << "Unexpected. File size is 27" << endl;
        }
        std::string mybuf;
        auto bytesRead = reader.readInto(mybuf, 10);
        if (bytesRead != 10) {
            cerr << "BytesRead should be 10";
            return -10;
        }

        if (mybuf.size() != 10) {
            cerr << "mybuf.size() shoule be 10";
            return -10;
        }
        if (mybuf != "ABCDEFGHIJ") {
            cerr << "my buf should have correct contents.";
            return -11;
        }
        if (reader.getPos() != 10) {
            cerr << "my buf position should be 10";
            return -12;
        }

        auto nowRead = reader.read(5);
        if (nowRead.size() != 5) {
            cerr << "Unexpected read result";
            return -20;
        }
        if (reader.getPos() != 15) {
            cerr << "reader position, expected 15";
            return -15;
        }

        if (reader.data().size() != 5) {
            cerr << "There should be a 5 byte buffer.";
            return -15;
        }

        // now, this should throw, but only once all the
        // data is exhausted.
        bool thrown = false;
        try {
            auto blowed_up = reader.read(1000);
            // cout << "actual bytes read = " << blowed_up << endl;
            blowed_up = reader.read(1);
        } catch (const std::exception&) {
            // cout << "Correctly caught " << e.what() << endl;
            thrown = true;
            auto posNow = reader.getPos();
            if (posNow != 27) {
                cerr << "Unexpected: file pos ought to be 27 hr";
                return -88;
            }
        }

        if (!thrown) {
            cerr << "Nope, it should have thrown with a crazy seek";
        }

        auto newpos = reader.seek(0);
        if (newpos != 0) {
            cerr << "unexpected: seek to 0 should work";
            return -1;
        }
        if (newpos != 0) {
            cerr << "newpos should be 0";
            return -1;
        }
        auto posback = reader.getPos();
        if (posback != 0) {
            cerr << "posback ought to be 0";
            return -1;
        }

        // seeking off the end behaviour
        auto seeked = reader.seek(10000);
        if (seeked != 10000) {
            cerr << "As mad as it is, seeking beyond the end of a file is ok!";
            return -1;
        }

        // check seeking backwards from the end
        seeked = reader.seek(0, std::ios_base::end);
        if (seeked != reader.getSize()) {
            cerr << "Expected a seek to the end of the file here.";
            return -1;
        }
        seeked = reader.seek(-2, std::ios_base::end);
        if (seeked != reader.getSize() - 2) {
            cerr << "Incorrect seek value returned";
            return -1;
        }
        auto two = reader.read(2);
        if (two.size() != 2) {
            cerr << "Expected two.";
            return -1;
        }

        std::string sbak;
        auto got = reader.readInto(sbak, 10);
        if (got != 0) {
            cerr << "read should get nothing since eof() is expected.";
            return -1;
        }

    } catch (const std::exception& e) {
        cerr << "UNEXPECTED failure opening file" << e.what() << endl;
        return -1;
    }

    return 0;
}

int main(int, char** argv) {

    cout << "Project running in: " << argv[0] << endl;
    int ret = test_file_reader();
    return ret;
}
