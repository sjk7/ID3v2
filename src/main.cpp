
#define CATCH_CONFIG_MAIN
#include "catch2KLJ.hpp"
#include "myUtils.hpp"
#include "FileDataReader.hpp"
#include "ID3v2.hpp"
using ID3v2::TagDataReader;

void ParseKnownBad() {
    const auto filePath = utils::find_file_up("testfile.txt", "ID3");
    REQUIRE_MESSAGE(!filePath.empty(), "testfile.txt needed for this test");
    TagDataReader parser(filePath, [](ID3v2::FrameHeaderEx&) {});
}

void ParseKnownGood() {
    using std::cout;
    using std::endl;
    const auto filePath = utils::find_file_up("sample.mp3", "ID3");
    REQUIRE_MESSAGE(!filePath.empty(), "sample.mp3 needed for this test");
    TagDataReader parser(filePath, [](ID3v2::FrameHeaderEx& f) {
        std::string nullstr(1, '\0');
        const auto d
            = utils::strings::replace_all_copy(f.AllData(), nullstr, "<NUL>");
        cout << "There is a tag, with ID: " << f.IDString()
             << ", length: " << ID3v2::FrameHeaderEx::GetFrameSize(f)
             << " bytes.\nData: " << d << endl;
    });
    cout << "ok" << endl;
}

TEST_CASE("ParseGoodFile") {
    REQUIRE_NOTHROW(ParseKnownGood());
}

// TEST_CASE("ParseShouldThrow") {
//     REQUIRE_THROWS_AS(ParseKnownBad(), std::runtime_error);
// }
