
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
    fs::path filePath;

    try {

        filePath = utils::find_file_up("testfile.txt", "ID3v2");
        REQUIRE_MESSAGE(!filePath.empty(), "testfile.txt needed for this test");
        TagDataReader parser(filePath.string(), [](ID3v2::FrameHeaderEx&) {});
    } catch (const std::exception&) {
        thrown = true;
    }
    REQUIRE(thrown);
    cout << "ParseKnownBad: OK -- exception was thrown as expected\nFor file: "
         << filePath << endl;
}

bool invalidChar(char c) {
    if (std::isspace(static_cast<unsigned char>(c))) return false;
    return !std::isalnum(static_cast<unsigned char>(c));
}
void stripUnicode(std::string& str) {
    str.erase(remove_if(str.begin(), str.end(), invalidChar), str.end());
}

void ParseKnownGood(const std::string& fileName) {
    using std::cout;
    using std::endl;

    const auto filePath = utils::find_file_up(fileName, "ID3v2");
    REQUIRE_MESSAGE(!filePath.empty(), fileName + " needed for this test");
    bool thrown = false;
    try {

        TagDataReader parser(filePath.string(), [](ID3v2::FrameHeaderEx& f) {
            std::string nullstr(1, '\0');
            std::string_view id{f.frameID, 4};
            if (id == "APIC") {
                cout << "This tag has an embedded image" << endl;
            } else {

                std::string mys = f.AllData();
                stripUnicode(mys);
                const auto& d = mys;

                cout << "There is a tag, with ID: " << f.IDString()
                     << ", length: " << ID3v2::FrameHeaderEx::GetFrameSize(f)
                     << " bytes.\nData: " << d << endl;
            }
        });
        cout << "ok. for filepath: " << filePath << endl;
    } catch (...) {
        thrown = true;
    }

    REQUIRE(!thrown);
}

void ParseAllInKnownGood(const std::string& fileName) {

    using std::cout;
    using std::endl;
    const auto filePath = utils::find_file_up(fileName, "ID3v2");
    REQUIRE_MESSAGE(!filePath.empty(), fileName + " needed for this test");
    ID3v2::TagCollection collection(filePath.u8string());
    REQUIRE(collection.Tags().size() > 0);
    const auto& tags = collection.Tags();

    cout << "----------------- All Tag Data in " << filePath.stem()
         << "-----------------\n\n";

    for (const auto& tag : tags) {
        const auto& ptr = tag.second;
        std::string rep = ptr->PayLoad();
        std::string uid = ptr->uid();
        if (uid.substr(0, 4) == "APIC") {
            rep = "<Image Data>";
        }

        cout << "tag: "
             << "uid: " << uid << " data: " << rep << endl;
    }
    cout << "\n\n" << endl;
}

TEST_CASE("DumpGoodFilesOutput") {

    SECTION("Parse Known good unicode tag") {
        ParseKnownGood("unicode-sample.mp3");
    }

    SECTION("Parse all tags in good sample file") {
        ParseAllInKnownGood("sampleSAVEDINDPS.mp3");
    }

    SECTION("Parse all tags in busted, but readable") {
        ParseAllInKnownGood("BustedButReadableTag.mp3");
    }

    SECTION("Parse all unicode tags in good sample file") {
        ParseAllInKnownGood("unicode-sample.mp3");
    }

    SECTION("Parse all tags in good sample file") {
        ParseAllInKnownGood("sample.mp3");
    }

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
