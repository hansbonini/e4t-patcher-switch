#include "utils.hpp"

#include <switch.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

#include "download.hpp"
#include "extract.hpp"
#include "fs.hpp"
#include "progress_event.hpp"
#include "constants.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

namespace util {

    bool isArchive(const std::string& path)
    {
        if (std::filesystem::exists(path)) {
            std::ifstream is(path, std::ifstream::binary);
            char zip_signature[4] = {0x50, 0x4B, 0x03, 0x04};  // zip signature header PK\3\4
            char signature[4];
            is.read(signature, 4);
            if (is.good() && std::equal(std::begin(signature), std::end(signature), std::begin(zip_signature), std::end(zip_signature))) {
                return true;
            }
        }
        return false;
    }

    void downloadArchive(const std::string& url, contentType type)
    {
        long status_code;
        downloadArchive(url, type, status_code);
    }

    void downloadArchive(const std::string& url, contentType type, long& status_code)
    {
        fs::createTree(fmt::format(DOWNLOAD_PATH, BASE_FOLDER_NAME));
        status_code = download::downloadFile(url, fmt::format(TRANSLATIONS_ZIP_PATH, BASE_FOLDER_NAME), OFF);
        ProgressEvent::instance().setStatusCode(status_code);
    }

    void showDialogBoxInfo(const std::string& text)
    {
        brls::Dialog* dialog;
        dialog = new brls::Dialog(text);
        brls::GenericEvent::Callback callback = [dialog](brls::View* view) {
            dialog->close();
        };
        dialog->addButton("menus/common/ok"_i18n, callback);
        dialog->setCancelable(true);
        dialog->open();
    }

    int showDialogBoxBlocking(const std::string& text, const std::string& opt)
    {
        int result = -1;
        brls::Dialog* dialog = new brls::Dialog(text);
        brls::GenericEvent::Callback callback = [dialog, &result](brls::View* view) {
            result = 0;
            dialog->close();
        };
        dialog->addButton(opt, callback);
        dialog->setCancelable(false);
        dialog->open();
        while (result == -1) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::microseconds(800000));
        return result;
    }

    int showDialogBoxBlocking(const std::string& text, const std::string& opt1, const std::string& opt2)
    {
        int result = -1;
        brls::Dialog* dialog = new brls::Dialog(text);
        brls::GenericEvent::Callback callback1 = [dialog, &result](brls::View* view) {
            result = 0;
            dialog->close();
        };
        brls::GenericEvent::Callback callback2 = [dialog, &result](brls::View* view) {
            result = 1;
            dialog->close();
        };
        dialog->addButton(opt1, callback1);
        dialog->addButton(opt2, callback2);
        dialog->setCancelable(false);
        dialog->open();
        while (result == -1) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::microseconds(800000));
        return result;
    }

    void extractArchive(contentType type)
    {
        chdir(ROOT_PATH);
        extract::extract(fmt::format(TRANSLATIONS_ZIP_PATH, BASE_FOLDER_NAME), AMS_CONTENTS);
    }

    std::string formatListItemTitle(const std::string& str, size_t maxScore)
    {
        size_t score = 0;
        for (size_t i = 0; i < str.length(); i++) {
            score += std::isupper(str[i]) ? 4 : 3;
            if (score > maxScore) {
                return str.substr(0, i - 1) + "\u2026";
            }
        }
        return str;
    }

    std::string downloadFileToString(const std::string& url)
    {
        std::vector<uint8_t> bytes;
        download::downloadFile(url, bytes);
        std::string str(bytes.begin(), bytes.end());
        return str;
    }

    void saveToFile(const std::string& text, const std::string& path)
    {
        std::ofstream file(path);
        file << text << std::endl;
    }

    std::string readFile(const std::string& path)
    {
        std::string text = "";
        std::ifstream file(path);
        if (file.good()) {
            file >> text;
        }
        return text;
    }

    std::string lowerCase(const std::string& str)
    {
        std::string res = str;
        std::for_each(res.begin(), res.end(), [](char& c) {
            c = std::tolower(c);
        });
        return res;
    }

    std::string upperCase(const std::string& str)
    {
        std::string res = str;
        std::for_each(res.begin(), res.end(), [](char& c) {
            c = std::toupper(c);
        });
        return res;
    }

    std::string getErrorMessage(long status_code)
    {
        std::string res;
        switch (status_code) {
            case 500:
                res = fmt::format("{0:}: Internal Server Error", status_code);
                break;
            case 503:
                res = fmt::format("{0:}: Service Temporarily Unavailable", status_code);
                break;
            default:
                res = fmt::format("error: {0:}", status_code);
                break;
        }
        return res;
    }

    bool isApplet()
    {
        AppletType at = appletGetAppletType();
        return at != AppletType_Application && at != AppletType_SystemApplication;
    }

    std::string getContentsPath()
    {
        return std::string(AMS_CONTENTS);
    }

    bool getBoolValue(const nlohmann::json& jsonFile, const std::string& key)
    {
        return (jsonFile.find(key) != jsonFile.end()) ? jsonFile.at(key).get<bool>() : false;
    }

    const nlohmann::ordered_json getValueFromKey(const nlohmann::ordered_json& jsonFile, const std::string& key)
    {
        return (jsonFile.find(key) != jsonFile.end()) ? jsonFile.at(key) : nlohmann::ordered_json::object();
    }

    bool getImageDimensions(const std::string& path, int& width, int& height)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.good()) return false;

        unsigned char header[32];
        file.read(reinterpret_cast<char*>(header), 32);
        if (!file.good()) return false;

        // PNG: signature + IHDR chunk: width at offset 16, height at offset 20, each 4 bytes big-endian
        if (header[0] == 0x89 && header[1] == 'P' && header[2] == 'N' && header[3] == 'G')
        {
            width = (header[16] << 24) | (header[17] << 16) | (header[18] << 8) | header[19];
            height = (header[20] << 24) | (header[21] << 16) | (header[22] << 8) | header[23];
            return width > 0 && height > 0;
        }

        // JPEG: search for SOF0 marker 0xFF 0xC0
        if (header[0] == 0xFF && header[1] == 0xD8)
        {
            file.seekg(2);
            unsigned char buf[32];
            while (file.read(reinterpret_cast<char*>(buf), 1))
            {
                if (buf[0] == 0xFF)
                {
                    file.read(reinterpret_cast<char*>(buf), 1);
                    if (!file.good()) break;
                    unsigned char marker = buf[0];
                    if (marker == 0xC0 || marker == 0xC1 || marker == 0xC2)
                    {
                        // SOF0, SOF1, SOF2: skip 3 bytes (precision), then height(2), width(2)
                        file.read(reinterpret_cast<char*>(buf), 7);
                        if (!file.good()) break;
                        height = (buf[3] << 8) | buf[4];
                        width = (buf[5] << 8) | buf[6];
                        return width > 0 && height > 0;
                    }
                    if (marker == 0xD9 || marker == 0xDA) break; // EOI/SOS — no SOF found
                    // Skip variable-length segment
                    file.read(reinterpret_cast<char*>(buf), 2);
                    if (!file.good()) break;
                    unsigned segLen = (buf[0] << 8) | buf[1];
                    file.seekg(file.tellg() + std::streamoff(segLen - 2));
                }
            }
        }

        return false;
    }

    void writeLog(std::string line)
    {
        std::ofstream logFile;
        logFile.open(fmt::format(LOG_FILE, BASE_FOLDER_NAME), std::ofstream::out | std::ofstream::app);
        if (logFile.is_open()) {
            logFile << line << std::endl;
        }
        logFile.close();
    }

    void doDelete(std::vector<std::string> folders)
    {
        if (!folders.empty())
        {
            ProgressEvent::instance().setTotalSteps(folders.size());
            ProgressEvent::instance().setStep(0);

            int index;
            std::string tmpFolder;
            std::string path = getContentsPath();

            for (std::string f : folders) {
                tmpFolder = f + "/";
                std::filesystem::remove_all(path + tmpFolder);
                index = tmpFolder.find_last_of("/");
                tmpFolder = tmpFolder.substr(0, index);

                while(tmpFolder.find_last_of("/") != std::string::npos)
                {
                    index = tmpFolder.find_last_of("/");
                    tmpFolder = tmpFolder.substr(0, index);

                    if (std::filesystem::exists(path + tmpFolder))
                    {
                        if (std::filesystem::is_empty(path + tmpFolder))
                            std::filesystem::remove_all(path + tmpFolder);
                    }
                }

                ProgressEvent::instance().incrementStep(1);
            }
        }
    }

    // trim from end of string (right)
    inline std::string& rtrim(std::string& s)
    {
        const char* t = "\t\n\r\f\v";
        s.erase(s.find_last_not_of(t) + 1);
        return s;
    }

    // trim from beginning of string (left)
    inline std::string& ltrim(std::string& s)
    {
        const char* t = "\t\n\r\f\v";
        s.erase(0, s.find_first_not_of(t));
        return s;
    }

    // trim from both ends of string (right then left)
    inline std::string& trim(std::string& s)
    {
        return ltrim(rtrim(s));
    }
}  // namespace util
