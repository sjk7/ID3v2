// WorkingMain.cpp
#include "myUtils.hpp"
#include "ID3v2.hpp"

int main(int argc, char** argv) {

    using std::cerr;
    using std::cout;
    using std::endl;

    fs::path srcDir = "H:\\audio-root-2023";
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

    bool extract_images = true;
    (void)extract_images;
    std::fstream fnotags(
        destDir / "These-have-no-tags.txt", std::ios::binary | std::ios::out);

    for (const fs::directory_entry& dir_entry :
        fs::recursive_directory_iterator(srcDir)) {
        bool threw = false;
        const fs::path p(dir_entry);
        const auto ext = p.filename().extension();
        if (!fs::is_directory(p)) {

            if (ext == ".mp3" || ext == ".MP3") {
                try {
                    // cout << "Reading: " << p.u8string() << endl;
                    ID3v2::TagCollection tags(p.u8string());
                    if (tags.hasNoV2Tags()) {
                        fnotags.write(p.u8string().data(), p.u8string().size());
                        if (tags.hasv1Tags()) {

                            const std::string v1dump = tags.dumpv1Tags();
                            fnotags.write(v1dump.data(), v1dump.size());
                        }
                        fnotags.write(NL.data(), NL.size());
                    }
                    // const auto pic = tags.pictureFrame();
                    // const auto& stem = p.stem();
                    /*/
                    if (pic != nullptr) {
                        auto picfn = (destDir / stem);
                        std::string picfp = picfn.u8string();
                        std::string extn
                            = ID3v2::ImageMimeExtension(pic->mimeType);
                        picfp += extn;
                        std::fstream fpic(
                            picfp, std::ios::out | std::ios::binary);
                        assert(fpic);
                        const auto& d = pic->payLoad;
                        fpic.write(d.data(), d.size());
                        assert(fpic);
                    }
                    /*/
                } catch (const std::exception& e) {
                    const std::string& sp(p.u8string());
                    flog.write(sp.data(), sp.size());
                    flog.write(TAB.data(), TAB.size());
                    flog.write(e.what(), strlen(e.what()));
                    flog.write(NL.data(), NL.size());
                    flog.flush();
                }
                threw = false;
            }
            if (threw) {
                // ... optionally move the file
            }
        }
    }
}
