#include <switch.h>

#include <borealis.hpp>
#include <filesystem>
#include <json.hpp>

#include "constants.hpp"
#include "fs.hpp"
#include "main_frame.hpp"
#include "utils.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

int main(int argc, char* argv[])
{
    // Init the app
    if (!brls::Application::init(APP_TITLE)) {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    // Apply E4T theme globally: purple background + white text for all screens
    {
        brls::LibraryViewsThemeVariantsWrapper* wrapper = brls::Application::getThemeVariantsWrapper();
        brls::Theme* themes[] = { wrapper->getLightTheme(), wrapper->getDarkTheme() };
        for (brls::Theme* theme : themes) {
            theme->backgroundColor[0] = 47.0f / 255.0f;
            theme->backgroundColor[1] = 2.0f / 255.0f;
            theme->backgroundColor[2] = 105.0f / 255.0f;
            theme->backgroundColorRGB = nvgRGB(47, 2, 105);
            theme->textColor          = nvgRGB(255, 255, 255);
            theme->descriptionColor   = nvgRGB(220, 220, 220);
            theme->separatorColor     = nvgRGBA(255, 255, 255, 76);
            theme->headerRectangleColor = nvgRGB(255, 255, 255);
        }
    }

    nlohmann::json languageFile = fs::parseJsonFile(fmt::format(LANGUAGE_JSON, BASE_FOLDER_NAME));
    if (languageFile.find("language") != languageFile.end())
        i18n::loadTranslations(languageFile["language"]);
    else
        i18n::loadTranslations();

        //appletInitializeGamePlayRecording();

        // Setup verbose logging on PC
#ifndef __SWITCH__
    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
#endif

    setsysInitialize();
    plInitialize(PlServiceType_User);
    nsInitialize();
    socketInitializeDefault();
    nxlinkStdio();
    pmdmntInitialize();
    pminfoInitialize();
    splInitialize();
    romfsInit();

    fs::createTree(fmt::format(CONFIG_PATH, BASE_FOLDER_NAME));

    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
    brls::Logger::debug("Start");

    brls::Application::pushView(new MainFrame());

    while (brls::Application::mainLoop());

    romfsExit();
    splExit();
    pminfoExit();
    pmdmntExit();
    socketExit();
    nsExit();
    setsysExit();
    plExit();
    return EXIT_SUCCESS;
}
