#pragma once

#include <switch.h>

#include <borealis.hpp>
#include <json.hpp>
#include <regex>
#include <set>

#include "constants.hpp"

namespace util {

    bool isArchive(const std::string& path);
    void downloadArchive(const std::string& url, contentType type);
    void downloadArchive(const std::string& url, contentType type, long& status_code);
    void extractArchive(contentType type);
    std::string formatListItemTitle(const std::string& str, size_t maxScore = 140);
    void showDialogBoxInfo(const std::string& text);
    int showDialogBoxBlocking(const std::string& text, const std::string& opt);
    int showDialogBoxBlocking(const std::string& text, const std::string& opt1, const std::string& opt2);
    std::string getErrorMessage(long status_code);
    std::string downloadFileToString(const std::string& url);
    void saveToFile(const std::string& text, const std::string& path);
    std::string readFile(const std::string& path);
    std::string lowerCase(const std::string& str);
    std::string upperCase(const std::string& str);
    std::string getContentsPath();
    bool getBoolValue(const nlohmann::json& jsonFile, const std::string& key);
    const nlohmann::ordered_json getValueFromKey(const nlohmann::ordered_json& jsonFile, const std::string& key);

/*
MY METHODS
*/

    bool getImageDimensions(const std::string& path, int& width, int& height);
    void writeLog(std::string line);
    void doDelete(std::vector<std::string> folders);
    std::string& rtrim(std::string& s);
    std::string& ltrim(std::string& s);
    std::string& trim(std::string& s);
}  // namespace util
