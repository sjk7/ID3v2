// WorkingMain.cpp
#include "myUtils.hpp"
#include "ID3v2.hpp"

int main(int argc, char** argv) {

    using std::cerr;
    using std::cout;
    using std::endl;

    fs::path srcDir = "H:\\audio-root-2018";
    fs::path destDir = "C:\\Users\\Cool\\Desktop\\BadID3Files";
    if (argc >= 2) {
        srcDir = argv[1];
    }
    if (argc >= 3) {
        destDir = argv[2];
    }
    if (!fs::is_directory(srcDir)) {
        cerr << srcDir.u8string()
             << " [input directory] is not a directory, cannot continue"
             << endl;
        return -1;
    }
    if (!fs::is_directory(destDir)) {
        cerr << destDir.u8string()
             << " [input directory] is not a directory, cannot continue"
             << endl;
        return -1;
    }

    std::fstream flog(destDir / "badones.txt",
        std::ios::out | std::ios::binary | std::ios::ate);
    assert(flog);
    const std::string NL = "\n";
    const std::string TAB = "\t";

    for (const fs::directory_entry& dir_entry :
        fs::recursive_directory_iterator(srcDir)) {
        bool threw = false;
        const fs::path p(dir_entry);
        const auto ext = p.filename().extension();
        if (!fs::is_directory(p)) {

            if (ext == ".mp3" || ext == ".MP3") {
                try {
                    cout << "Reading: " << p.u8string() << endl;
                    ID3v2::TagCollection tags(p.u8string());
                } catch (const std::exception& e) {
                    const std::string& sp(p.u8string());
                    flog.write(sp.data(), sp.size());
                    flog.write(TAB.data(), TAB.size());
                    flog.write(e.what(), strlen(e.what()));
                    flog.write(NL.data(), NL.size());
                    flog.flush();
                    if (sp
                        == "H:\\audio-root-2023\\80S\\2\\Opener\\PRINCE - I "
                           "WANNA BE YOUR LOVER.mp3") {
                    } else {

                        threw = true;
                    }
                }
                threw = false;
            }
            if (threw) {
                // ... optionally move the file
            }
        }
    }
}
