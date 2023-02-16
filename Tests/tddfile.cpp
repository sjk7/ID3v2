// tddfile.cpp
#include "../include/myUtils.hpp"
using namespace std;
using namespace utils;

int tdd_file()
{
    {
        // try a bad file path
        try
        {
            std::fstream f;
            utils::file_open(f, "", std::ios::binary | std::ios::in, true);
        }
        catch (const std::system_error &)
        {
            // std::cout << e.what() << "|" << e.code() << endl;
            // fall through
        }
        catch (const std::exception &)
        {
            cerr << "Failed test: was expecting an error opening a file with "
                    "no path"
                 << endl;
            return -1;
        }

        // try opening a DIRECTORY
        std::fstream f;
        try
        {
            utils::file_open(f, ".");
        }
        catch (const std::system_error &)
        {
            // std::cerr << e.what() << " | " << e.code() << endl;
        }
        catch (const std::exception &)
        {
            cerr << "Failed test: was expecting an error opening a file with "
                    "no path"
                 << endl;
            return -1;
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
        try
        {
            utils::file_open(f, filepath);
        }
        catch (const std::system_error &)
        {
            // std::cout << e.what() << " | " << e.code() << endl;
        }
        catch (const std::exception &)
        {
            cerr << "Failed test: was expecting an error opening a file with "
                    "no path"
                 << endl;
            return -1;
        }

        // read from a known bad file
        try
        {
            assert(!f);
            std::string s;
            auto nRead = file_read_some(s, f, 10, ".");
            assert(nRead == 0);
            (void)nRead;
        }
        catch (const std::exception &e)
        {
            std::string s = e.what();
            // std::cerr << e.what() << endl;
            if (s.find("is not open") == std::string::npos)
            {
                return -1;
            }
        }
    }

    {
        std::fstream f;
        try
        {
            std::string testFilePath = utils::find_file_up("testfile.txt", "ID3v2");
            if (testFilePath.empty())
            {
#ifdef _MSC_VER
#pragma warning(disable : 4130)
#endif
                assert("testfile.txt is not present in the working directory "
                       "or any of the local directories above" == 0);
            }

            utils::file_open(f, testFilePath);

            assert(f.is_open());
            std::string data;
            auto read1 = file_read_some(data, f, 10, testFilePath);
            assert(read1 == 10);
            assert(data == "ABCDEFGHIJ");

            // very nice, let's read some more and read off the end of the file:
            auto read2 = file_read_some(data, f, 100, testFilePath);
            assert(read2 == 17);
            bool ok = false;
            const auto sz = file_get_size(f, ok);
            if (!ok)
                return -2;
            assert(ok);
            assert(read1 + read2 == (std::ptrdiff_t)sz);
            if (read1 + read2 != (std::ptrdiff_t)sz)
                return -3;
            if (!f.eof())
                return -4;
            assert(f.eof());
            auto pos = 0;
            auto where_to = file_seek(pos, f, ok, std::ios_base::beg);
            assert(where_to == 0);
            if (where_to != 0)
                return -5;
            assert(ok);
            if (!ok)
                return -6;
            assert(f);
            if (!f)
                return -7;
        }
        catch (const std::exception &e)
        {
            cerr << "Bad news, an expected operation failed with: " << endl;
            cerr << e.what();
            return -1;
        }
    }

    return 0;
}

int test_reader()
{
    return 0;
}

int main(int, char **argv)
{
    std::cout << "Tddfile running in " << argv[0] << endl;
    int ret = tdd_file();
    if (ret)
        return ret;
    ret = test_reader();
    return ret;
}
