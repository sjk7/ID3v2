
#define CATCH_CONFIG_MAIN
#include "catch2KLJ.hpp"
#include "myUtils.hpp"
#include "FileDataReader.hpp"
#include "ID3v2.hpp"

struct TagDataReader {
    template <typename CB>
    TagDataReader(const std::string& filePath, CB&& cb) : m_dr(filePath) {
        m_info.filePath = filePath;
        m_info.totalFileSize = m_dr.getSize();
        m_info.tag = ID3v2::parseHeader(m_dr, filePath);
        if (m_info.tag.validity != ID3v2::verifyTagResult::OK) {
            throw std::runtime_error("File: " + filePath + " has no ID3v2Tags");
        }

        ID3v2::FillTags(m_info, m_dr, std::forward<CB>(cb));
    }

    my::FileDataReader m_dr;
    ID3v2::ID3FileInfo m_info = {};
};

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
        cout << "There is a tag, with ID:" << f.IDString()
             << ", length: " << ID3v2::FrameHeaderEx::GetFrameSize(f)
             << " bytes." << endl;
    });
    cout << "ok" << endl;
}

TEST_CASE("ParseGoodFile") {
    REQUIRE_NOTHROW(ParseKnownGood());
}

// TEST_CASE("ParseShouldThrow") {
//     REQUIRE_THROWS_AS(ParseKnownBad(), std::runtime_error);
// }
