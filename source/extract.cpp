#include "extract.hpp"

#include <unzipper.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "download.hpp"
#include "fs.hpp"
#include "main_frame.hpp"
#include "progress_event.hpp"
#include "utils.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

namespace extract {

    namespace {
        void preWork(zipper::Unzipper& unzipper, const std::string& workingPath, std::vector<zipper::ZipEntry>& entries)
        {
            if (!std::filesystem::exists(workingPath))
                std::filesystem::create_directory(workingPath);

            chdir(workingPath.c_str());
            entries = unzipper.entries();
            s64 uncompressedSize = 0;
            s64 freeStorage;
            for (const auto& entry : entries)
                uncompressedSize += entry.uncompressedSize;

            if (R_SUCCEEDED(fs::getFreeStorageSD(freeStorage))) {
                if (uncompressedSize * 1.1 > freeStorage) {
                    unzipper.close();
                    brls::Application::crash("menus/errors/insufficient_storage"_i18n);
                    std::this_thread::sleep_for(std::chrono::microseconds(2000000));
                    brls::Application::quit();
                }
            }
            ProgressEvent::instance().setTotalSteps(entries.size() + 1);
            ProgressEvent::instance().setStep(0);
        }
    }  // namespace

    void extract(const std::string& filename, const std::string& workingPath, int overwriteInis, std::function<void()> func)
    {
        zipper::Unzipper unzipper(filename);
        std::vector<zipper::ZipEntry> entries;

        preWork(unzipper, workingPath, entries);
        std::set<std::string> ignoreList = fs::readLineByLine(fmt::format(FILES_IGNORE, BASE_FOLDER_NAME));

        for (const auto& entry : entries) {
            if (ProgressEvent::instance().getInterupt()) {
                break;
            }
            if ((overwriteInis == 0 && entry.name.substr(entry.name.length() - 4) == ".ini") || find_if(ignoreList.begin(), ignoreList.end(), [&entry](std::string ignored) {
                                                            u8 res = ("/" + entry.name).find(ignored);
                                                            return (res == 0 || res == 1); }) != ignoreList.end()) {
                if (!std::filesystem::exists("/" + entry.name)) {
                    unzipper.extractEntry(entry.name);
                }
            }
            else if (entry.name == "atmosphere/stratosphere.romfs" || entry.name == "atmosphere/package3") {
                std::ofstream readonlyFile(entry.name + ".apg");
                unzipper.extractEntryToStream(entry.name, readonlyFile);
            }

            else if (
                        (workingPath == "/") &&
                        (
                            entry.name == "switch/tinfoil/credentials.json" || entry.name == "switch/tinfoil/gdrive.token" || entry.name == "switch/tinfoil/locations.conf" || entry.name == "switch/tinfoil/options.json" ||
                            entry.name.find("switch/tinfoil/themes") != std::string::npos
                        )
                    )
            {
                continue;
            }
            else {
                unzipper.extractEntry(entry.name);
                if (entry.name.substr(0, 13) == "hekate_ctcaer") {
                    fs::copyFile("/" + entry.name, UPDATE_BIN_PATH);
                }
            }
            ProgressEvent::instance().incrementStep(1);
        }
        unzipper.close();
        ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
    }


}  // namespace extract