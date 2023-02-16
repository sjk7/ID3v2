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
        bool ok = false;
        auto newpos = reader.seek(0, ok);
        if (!ok) {
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
        auto seeked = reader.seek(10000, ok);
        if (!ok) {
            cerr << "As mad as it is, seeking beyond the end of a file is ok!";
            return -1;
        }
        assert(seeked == 10000);

        // check seeking backwards from the end
        seeked = reader.seek(0, ok, std::ios_base::end);
        assert(seeked == 27);
        seeked = reader.seek(-2, ok, std::ios_base::end);
        assert(seeked = 25);
        auto two = reader.read(2);
        assert(two.size() == 2);

        std::string sbak;
        auto got = reader.readInto(sbak, 10);
        assert(got == 0); // coz eof, and no exception here
    } catch (const std::exception& e) {
        cerr << "UNEXPECTED failure opening file" << e.what() << endl;
        return -1;
    }

    return 0;
}

int main(int, char** argv) {

    cout << "Project running in: " << argv[0] << endl;
    int ret = test_file_reader();
    return 0;
}
