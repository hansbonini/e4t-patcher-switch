#include "main_frame.hpp"

#include <filesystem>
#include <fstream>
#include <json.hpp>

#include "fs.hpp"
#include "list_extra_tab.hpp"
#include "utils.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;
using json = nlohmann::json;

namespace {
    constexpr const char AppTitle[] = APP_TITLE;
    constexpr const char AppVersion[] = APP_VERSION;
}

MainFrame::MainFrame() : AppletFrame(true, true)
{
    this->setIcon("romfs:/gui_icon.png");
    this->setTitle(AppTitle);

    s64 freeStorage;

    this->setFooterText(fmt::format("menus/main/footer_text"_i18n, BRAND_FULL_NAME,
        "v" + std::string(AppVersion),
        R_SUCCEEDED(fs::getFreeStorageSD(freeStorage)) ? floor(((float)freeStorage / 0x40000000) * 100.0) / 100.0 : -1));

    // Ensure translations.json exists on SD — copy from ROMFS on first run
    std::string translationsPath = fmt::format("/config/{}/translations.json", BASE_FOLDER_NAME);
    if (!std::filesystem::exists(translationsPath)) {
        fs::createTree(fmt::format("/config/{}/", BASE_FOLDER_NAME));
        fs::copyFile("romfs:/translations.json", translationsPath);
    }

    // Load translations from local SD file
    nlohmann::ordered_json nxlinks = fs::parseJsonFile(translationsPath);

    this->setContentView(new ListExtraTab(contentType::translations, nxlinks));

    this->registerAction("", brls::Key::B, [this] { return true; });
}

void MainFrame::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    // Solid purple background fill
    nvgBeginPath(vg);
    nvgRect(vg, x, y, width, height);
    nvgFillColor(vg, nvgRGBA(47, 2, 105, 255));
    nvgFill(vg);

    // Load and draw centered logo
    if (bgImage == 0) {
        bgImage = nvgCreateImage(vg, "romfs:/background.jpg", 0);
    }

    if (bgImage != 0) {
        int imgW, imgH;
        nvgImageSize(vg, bgImage, &imgW, &imgH);
        if (imgW > 0 && imgH > 0) {
            float scale = fmin((float)width / (float)imgW, (float)height / (float)imgH) * 0.5f;
            float drawW = imgW * scale;
            float drawH = imgH * scale;
            float drawX = x + (width - drawW) / 2.0f;
            float drawY = y + (height - drawH) / 2.0f;
            NVGpaint imgPaint = nvgImagePattern(vg, drawX, drawY, drawW, drawH, 0.0f, bgImage, 1.0f);
            nvgBeginPath(vg);
            nvgRect(vg, drawX, drawY, drawW, drawH);
            nvgFillPaint(vg, imgPaint);
            nvgFill(vg);
        }
    }

    // Override layout values: align header icon/title and footer text with separator (no extra left padding)
    unsigned savedImageLeftPadding = style->AppletFrame.imageLeftPadding;
    unsigned savedTitleStart = style->AppletFrame.titleStart;
    unsigned savedFooterTextSpacing = style->AppletFrame.footerTextSpacing;

    style->AppletFrame.imageLeftPadding = style->AppletFrame.separatorSpacing;
    style->AppletFrame.titleStart = style->AppletFrame.separatorSpacing + style->AppletFrame.imageSize + 14;
    style->AppletFrame.footerTextSpacing = 0;

    // Run layout with updated imageLeftPadding to reposition icon at separator
    this->layout(vg, style, ctx->fontStash);

    // Set theme colors to white for header/footer, then draw, then restore
    NVGcolor savedTextColor = ctx->theme->textColor;
    NVGcolor savedSeparatorColor = ctx->theme->separatorColor;
    ctx->theme->textColor = nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f);
    ctx->theme->separatorColor = nvgRGBAf(1.0f, 1.0f, 1.0f, 0.3f);

    // Draw AppletFrame chrome and content on top
    AppletFrame::draw(vg, x, y, width, height, style, ctx);

    ctx->theme->textColor = savedTextColor;
    ctx->theme->separatorColor = savedSeparatorColor;

    // Restore layout values
    style->AppletFrame.imageLeftPadding = savedImageLeftPadding;
    style->AppletFrame.titleStart = savedTitleStart;
    style->AppletFrame.footerTextSpacing = savedFooterTextSpacing;
}
