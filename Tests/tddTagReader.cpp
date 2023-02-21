
#define CATCH_CONFIG_MAIN
#include "catch2KLJ.hpp"

#include "FileDataReader.hpp"
#include "ID3v2.hpp"
#include "myUtils.hpp"

using ID3v2::TagDataReader;

TEST_CASE("ParseKnownBad") {

    using std::cout;
    using std::endl;
    bool thrown = false;
    fs::path filePath;

    try {

        filePath = utils::find_file_up("testfile.bin", "ID3v2");
        REQUIRE_MESSAGE(!filePath.empty(), "testfile.bin needed for this test");
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
    if (c == ':') return false;
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
    using std::wcout;
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
        //
        const auto unicode = ptr->textEncoding;
        std::string uid = ptr->uid();

        if (uid.substr(0, 4) == "APIC") {
            rep = "<Image Data>";
        }

        stripUnicode(uid);
        cout << "tag: "
             << "uid: " << uid;
        if (unicode == ID3v2::TextEncodingType::unicode_with_bom) {
            cout << " *********** UNICODE VALUE *************" << endl;
            rep = rep.substr(2);
        }
        stripUnicode(rep);
        cout << rep << endl;
        if (ptr->IsMalformed()) {
            cout << "NOTE: This tag appears to be malformed." << endl;
        }
    }
    cout << "\n\n" << endl;

    cout << "----- v1 tags -----" << endl;
    const auto& tag = collection.info.Tag();

    const auto art = tag.v1Artist();
    const auto tit = tag.v1Title();
    const auto alb = tag.v1Album();
    const auto comment = tag.v1Comment();
    const auto yr = tag.v1Year();
    const auto nyear = tag.v1year();
    const auto genre = tag.v1Genre();
    cout << genre << endl;
    cout << art << endl;
    cout << tit << endl;
    cout << alb << endl;
    cout << yr << endl;
    cout << comment << endl;
    cout << nyear << endl;

    if (fileName == "sampleSAVEDINDPS.mp3") {

        const auto sec = collection.UserTag("Sec Tone");
        cout << "Sectone is: " << sec << endl;
        cout << endl;
        REQUIRE(art == "This Is The Artist Field");
        REQUIRE(tit == "This Is The Title Field");
        REQUIRE(alb == "");
        REQUIRE(genre.empty());
        REQUIRE(comment == "");
        REQUIRE(yr == "2023");
        REQUIRE(nyear == 2023);
        const auto v2Art = collection.Artist();
        cout << v2Art << endl;
        REQUIRE(v2Art == "This Is The Artist Field");
        const auto v2Title = collection.Title();
        cout << v2Title << endl;
        REQUIRE(v2Title == "This Is The Title Field");

        const auto v2Album = collection.Album();
        cout << v2Album << endl;
        REQUIRE(v2Album == "This is the album title");
    } else if (fileName == "BustedButReadableTag.mp3") {
        REQUIRE(art == "Twinkle");
        REQUIRE(tit == "Terry");
        REQUIRE(alb == "");
        REQUIRE(comment == "");
        REQUIRE(nyear == 1964);
        REQUIRE(yr == "1964");
        REQUIRE(genre == "Blues");
    } else if (fileName == "sample.mp3") {
        REQUIRE(genre == "Bebop");
    }
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
