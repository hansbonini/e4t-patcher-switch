#pragma once
/*
CHANGE ONLY THESE CONSTANTS
*/
constexpr const char BRAND_ARTICLE[] = "um";
constexpr const char BRAND_FULL_NAME[] = "E4T Patcher";
constexpr const char BRAND_FACEBOOK_GROUP[] = "";
constexpr const char APP_FULL_NAME[] = "E4T Patcher";
constexpr const char APP_SHORT_NAME[] = "E4T Patcher";
constexpr const char BASE_FOLDER_NAME[] = "e4t-patcher";
constexpr const char GITHUB_USER[] = "";
constexpr const bool SHOW_GNX = false;

/*
DO NOT CHANGE ONLY THESE CONSTANTS UNLESS YOU KNOW WHAT YOU'RE DOING
*/

constexpr const bool DEBUG = false;

constexpr const char CFW_ROOT_PATH[] = "/apgtmppackfolder";
constexpr const char ROOT_PATH[] = "/";
constexpr const char DOWNLOAD_PATH[] = "/config/{}/";
constexpr const char CONFIG_PATH[] = "/config/{}/";

constexpr const char TRANSLATIONS_URL[] = "https://raw.githubusercontent.com/hansbonini/e4t-patcher-switch/refs/heads/main/resources/translations.json";

constexpr const char AMS_CONTENTS[] = "/atmosphere/contents/";
constexpr const char AMS_PATH[] = "/atmosphere/";
constexpr const char CONTENTS_PATH[] = "contents/";

constexpr const char TRANSLATIONS_ZIP_PATH[] = "/config/{}/translations.zip";
constexpr const char IMAGE_CACHE_PATH[] = "/config/{}/cache/";

constexpr const char LANGUAGE_JSON[] = "/config/{}/language.json";

constexpr const char HIDDEN_APG_FILE[] = "/config/{}/.{}";

constexpr const char FILES_IGNORE[] = "/config/{}/preserve.txt";
constexpr const char UPDATE_BIN_PATH[] = "/bootloader/update.bin";

constexpr const char LOG_FILE[] = "/config/{}/log.txt";

constexpr const int LISTITEM_HEIGHT = 50;

constexpr const int MAX_FETCH_LINKS = 15;

enum class contentType
{
    translations
};

constexpr std::string_view contentTypeNames[1]{"translations"};
constexpr std::string_view contentTypeFullNames[1]{"a tradução"};

enum class DialogType
{
    updating,
    warning,
    error,
    beta
};

constexpr std::string_view ROMFSIconFile[4]{"romfs:/updating.png", "romfs:/warning.png", "romfs:/error.png", "romfs:/beta.png"};

enum class CFW
{
    rnx,
    sxos,
    ams,
};
