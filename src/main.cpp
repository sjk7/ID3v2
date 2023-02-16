#include "ID3v2.hpp"
#include <iostream>
using namespace std;

// Convenience class for when you are not interested
// in the actual tags, but you want a reader opened beyond
// the id3v2Tag (say, if you are an audio parser), and
// to populate where the audio ends, as well as where it starts.
// V1 tags are taken  into consideration here.
struct ID3v2Skipper
{

    template <typename F> ID3v2Skipper(const std::string &filePath, F &&f) : m_dr(filePath)
    {
        m_info.tag = ID3v2::parseHeader(m_dr);
        const auto fsize = m_dr.getSize();
        m_info.getMpegSize(fsize);
        bool ok = false;
        m_dr.seek(m_info.tag.totalSizeInBytes(), ok);
        f(m_dr, m_info);
    }

    ID3v2::ID3FileInfo m_info = {};

    std::string m_filePath;
    my::FileDataReader m_dr;
};

int main()
{
    std::string s("\tAmple\topportunity\t\t\for\tfuckups\t\t");
    auto tokens = utils::strings::split_string_gpt(s, "\t", true);
    cout << "nTokens = " << tokens.size() << endl;
    for (const auto& tk : tokens){
        cout << tk << endl;
    }
    cout << "-----------------" << endl;

    tokens= utils::strings::split_string_gpt(s, "\t");
    cout << "nTokens = " << tokens.size() << endl;
        for (const auto& tk : tokens){
        cout << tk << endl;
    }
 cout << "-----------------" << endl;
 std::string sNoTokens = "sdpkjghapiughladikfnpcadisuofynavsidoviu";
 tokens = utils::strings::split_string_gpt(sNoTokens, "\t");
     cout << "nTokens = " << tokens.size() << endl;
        for (const auto& tk : tokens){
        cout << tk << endl;
    }
 cout << "-----------------" << endl;



    const auto filePath = utils::find_file_up("sample.mp3", "ID3");
    assert(!filePath.empty());

    ID3v2Skipper skipper(filePath, [](my::FileDataReader &rdr, ID3v2::ID3FileInfo &info) {
        cout << "Header size is:" << info.tag.dataSizeInBytes << endl;
    });

    auto &dr = skipper.m_dr;
    std::string mpegData;
    const auto filesize = dr.getSize();
    size_t mpegSize = skipper.m_info.mpegSize;
    bool ok = false;
    auto seekpos = skipper.m_info.tag.totalSizeInBytes();
    auto seeked = dr.seek(seekpos, ok);
    auto got = dr.readInto(mpegData, mpegSize);
    assert(got == mpegSize);
    cout << "Here's some mpeg data (could be padding thou) ... \n\n" << mpegData << endl;
    unsigned char c1 = mpegData[0];
    unsigned char c2 = mpegData[1];
    cout << "First two bytes are " << (int)c1 << " " << (int)c2 << endl;
}