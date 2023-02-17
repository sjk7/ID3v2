// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

// tddfile.cpp
#define CATCH_CONFIG_MAIN
#include "../include/catch2KLJ.hpp"
#include "../include/myUtils.hpp"

using namespace std;
using namespace utils;

TEST_CASE("Checking file behaviours") {
    {
        // try a bad file path
        // REQUIRE(false);
        try {
            std::fstream f;
            utils::file_open(f, "", std::ios::binary | std::ios::in, true);
        } catch (const std::system_error& e) {
            REQUIRE(e.what());
            // std::cout << e.what() << "|" << e.code() << endl;
            // fall through
        } catch (const std::exception& err) {
            cerr << "Failed test: was expecting an error opening a file with "
                    "no path"
                 << endl;
            REQUIRE(err.what() == nullptr);
            // return -1;
        }

        // try opening a DIRECTORY
        std::fstream f;
        try {
            utils::file_open(f, ".");
        } catch (const std::system_error& e) {
            // std::cerr << e.what() << " | " << e.code() << endl;
            const std::string s{e.what()};
            REQUIRE(!s.empty());
        } catch (const std::exception&) {
            cerr << "Failed test: was expecting an error opening a file with "
                    "no path"
                 << endl;
            std::string bollo
                = "FAIL because should have caught system_error when opening "
                  "a file with no path";
            REQUIRE(bollo.empty());
        }

        // try opening something that we should never have permission to read:
    }
    {
#ifdef _WIN32
        const auto filepath = "C:\\windows\\system32\\myunreadable.txt";
#else
        const auto filepath = "/private/var/protected/xprotect/XPdb";
#endif
        std::fstream f;
        try {
            utils::file_open(f, filepath);
        } catch (const std::system_error&) {
            // std::cout << e.what() << " | " << e.code() << endl;
        } catch (const std::exception& err) {
            REQUIRE(err.what() == nullptr);
            // << endl;
            // return -1;
        }

        // read from a known bad file
        try {
            assert(!f);
            std::string s;
            auto nRead = file_read_some(s, f, 10, ".");
            REQUIRE(nRead == 0);
        } catch (const std::exception& e) {
            std::string s = e.what();
            REQUIRE(s.find("is not open") != std::string::npos);
        }
    }

    {
        std::fstream f;
        try {
            std::string testFilePath
                = utils::find_file_up("testfile.txt", "ID3v2");
            REQUIRE(!testFilePath.empty());

            utils::file_open(f, testFilePath);

            REQUIRE(f.is_open());
            std::string data;
            auto read1 = file_read_some(data, f, 10, testFilePath);
            REQUIRE(read1 == 10);
            REQUIRE(data == "ABCDEFGHIJ");

            // very nice, let's read some more and read off the end of the file:
            auto read2 = file_read_some(data, f, 100, testFilePath);
            assert(read2 == 17);
            bool ok = false;
            const auto sz = file_get_size(f, ok);
            REQUIRE(ok);
            REQUIRE(read1 + read2 == (std::ptrdiff_t)sz);

            REQUIRE(f.eof());
            auto pos = 0;
            auto where_to = file_seek(pos, f, std::ios_base::beg);
            REQUIRE(where_to == 0);
            REQUIRE(ok);
            REQUIRE(f);
        } catch (const std::exception& e) {
            cerr << "Bad news, an expected operation failed with: " << endl;
            cerr << e.what();
            REQUIRE(false);
            // return -1;
        }
    }

    //  char buf[2] = {};
    //  volatile const auto wrong = buf[8];
    //  cout << wrong << endl;
}

int test_reader() {
    return 0;
}

/*/
int main(int, char **argv)
{
    std::cout << "Tddfile running in " << argv[0] << endl;
    int ret = tdd_file();
    if (ret)
        return ret;
    ret = test_reader();
    return ret;
}
/*/
