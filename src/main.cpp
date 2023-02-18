
#define CATCH_CONFIG_MAIN
#include "catch2KLJ.hpp"

#include "myUtils.hpp"
#include "FileDataReader.hpp"
#include "ID3v2.hpp"
using ID3v2::TagDataReader;

TEST_CASE("ParseKnownBad") {

    using std::cout;
    using std::endl;
    bool thrown = false;
    std::string filePath;
    try {

        filePath = utils::find_file_up("testfile.txt", "ID3v2");
        REQUIRE_MESSAGE(!filePath.empty(), "testfile.txt needed for this test");
        TagDataReader parser(filePath, [](ID3v2::FrameHeaderEx&) {});
    } catch (const std::exception&) {
        thrown = true;
    }
    REQUIRE(thrown);
    cout << "ParseKnownBad: OK -- exception was thrown as expected\nFor file: "
         << filePath << endl;
}

void ParseKnownGood(const std::string& fileName) {
    using std::cout;
    using std::endl;

    const auto filePath = utils::find_file_up(fileName, "ID3v2");
    REQUIRE_MESSAGE(!filePath.empty(), fileName + " needed for this test");
    bool thrown = false;
    try {

        TagDataReader parser(filePath, [](ID3v2::FrameHeaderEx& f) {
            std::string nullstr(1, '\0');
            const auto d = utils::strings::replace_all_copy(
                f.AllData(), nullstr, "<NUL>");
            cout << "There is a tag, with ID: " << f.IDString()
                 << ", length: " << ID3v2::FrameHeaderEx::GetFrameSize(f)
                 << " bytes.\nData: " << d << endl;

            std::string_view id{f.frameID, 4};
            if (id == "APIC") {
                cout << "This tag has an embedded image" << endl;
            }
        });
        cout << "ok. for filepath: " << filePath << endl;
    } catch (...) {
        thrown = true;
    }

    REQUIRE(!thrown);
}

TEST_CASE("DumpGoodFilesOutput") {

    SECTION("Known good file: sample.mp3") {
        ParseKnownGood("sample.mp3");
    }
    SECTION("Busted internally, but tag readable: BustedButReadableTag.mp3") {
        ParseKnownGood("BustedButReadableTag.mp3");
    }
    SECTION("Some Tags that were saved in DPS: sampleSAVEDINDPS.mp3") {
        ParseKnownGood("sampleSAVEDINDPS.mp3");
    }
}
