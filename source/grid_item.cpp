#include "grid_item.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include <webp/decode.h>

#include "confirm_page.hpp"
#include "list_download_tab_confirmation.hpp"
#include "download.hpp"
#include "utils.hpp"
#include "worker_page.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

GridItem::GridItem(std::string title, std::string size, bool installed, std::string url, contentType type,
                   std::vector<std::string> itemFolders, bool bInstalled,
                   std::string imagePath, int imgW, int imgH)
    : title(std::move(title)), size(std::move(size)), installed(installed), url(std::move(url)),
      type(type), itemFolders(std::move(itemFolders)), bInstalled(bInstalled),
      imagePath(std::move(imagePath)), imgW(imgW), imgH(imgH)
{
    if (this->imgW > 0 && this->imgH > 0) {
        this->cardHeight = std::min(CARD_WIDTH * this->imgH / this->imgW, MAX_CARD_HEIGHT);
    } else {
        this->cardHeight = DEFAULT_CARD_HEIGHT;
    }

    this->setWidth(CARD_WIDTH);
    this->setHeight(this->cardHeight + LABEL_HEIGHT + TOTAL_PADDING);

    this->registerAction(this->bInstalled ? "Desinstalar Tradução" : "Instalar Tradução", brls::Key::A, [this] {
        this->clickEvent.fire(this);
        return true;
    });
}

void GridItem::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    int cardDrawY = y + 2;
    int cardBottom = cardDrawY + cardHeight;
    int labelAreaTop = cardBottom + 4;
    int labelCenterY = labelAreaTop + LABEL_HEIGHT / 2;

    // Draw image inside card (if available)
    if (!imagePath.empty() && !imageLoaded) {
        imageHandle = loadImage(vg, imagePath);
        imageLoaded = true;
    }

    if (imageHandle != 0) {
        int loadedW, loadedH;
        nvgImageSize(vg, imageHandle, &loadedW, &loadedH);
        if (loadedW > 0 && loadedH > 0) {
            // Scale image to fill card edge-to-edge, maintaining aspect ratio, centered
            float scale = fmin((float)width / (float)loadedW, (float)cardHeight / (float)loadedH);
            float imgDrawW = loadedW * scale;
            float imgDrawH = loadedH * scale;
            float imgDrawX = x + ((float)width - imgDrawW) / 2.0f;
            float imgDrawY = cardDrawY + ((float)cardHeight - imgDrawH) / 2.0f;

            NVGpaint imgPaint = nvgImagePattern(vg, imgDrawX, imgDrawY, imgDrawW, imgDrawH, 0.0f, imageHandle, 1.0f);
            nvgBeginPath(vg);
            nvgRect(vg, x, cardDrawY, width, cardHeight);
            nvgFillPaint(vg, imgPaint);
            nvgFill(vg);
        }
    }

    // Installed star (top-left of card)
    if (installed) {
        nvgFontSize(vg, 14);
        nvgFontFaceId(vg, ctx->fontStash->regular);
        nvgFillColor(vg, nvgRGBA(255, 215, 0, 255));
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgText(vg, x + 6, cardDrawY + 4, "\u2605", nullptr);
    }

    // Title [size] below card (centered, in the LABEL_HEIGHT area)
    nvgFontSize(vg, 10);
    nvgFontFaceId(vg, ctx->fontStash->regular);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    int labelCenterX = x + width / 2;
    float labelMaxWidth = width - 8;
    std::string labelStr = title + " [" + size + "]";
    float lw = nvgTextBounds(vg, 0, 0, labelStr.c_str(), nullptr, nullptr);
    if (lw > labelMaxWidth) {
        char buf[256] = {0};
        size_t len = labelStr.length();
        for (size_t i = 0; i < len && i < 254; i++) {
            buf[i] = labelStr[i];
            buf[i + 1] = '\0';
            float w = nvgTextBounds(vg, 0, 0, buf, nullptr, nullptr);
            if (w > labelMaxWidth - 10) {
                if (i > 2) {
                    buf[i - 2] = '.';
                    buf[i - 1] = '.';
                    buf[i] = '\0';
                }
                break;
            }
        }
        nvgText(vg, labelCenterX, labelCenterY, buf, nullptr);
    } else {
        nvgText(vg, labelCenterX, labelCenterY, labelStr.c_str(), nullptr);
    }
}

void GridItem::frame(brls::FrameContext* ctx)
{
    // Save original highlight colors
    NVGcolor origC1 = ctx->theme->highlightColor1;
    NVGcolor origC2 = ctx->theme->highlightColor2;

    // Use white highlight border for the grid cards
    ctx->theme->highlightColor1 = nvgRGBA(255, 255, 255, 255);
    ctx->theme->highlightColor2 = nvgRGBA(255, 255, 255, 255);

    // Draw highlight with white colors via parent
    brls::View::frame(ctx);

    // Restore original theme colors
    ctx->theme->highlightColor1 = origC1;
    ctx->theme->highlightColor2 = origC2;
}

void GridItem::layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash)
{
    // Layout handled by parent BoxLayout
}

brls::View* GridItem::getDefaultFocus()
{
        return this;
}

int GridItem::loadImage(NVGcontext* vg, const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) return 0;

    char header[12];
    file.read(header, 12);
    file.close();

    bool isWebP = (std::memcmp(header, "RIFF", 4) == 0 && std::memcmp(header + 8, "WEBP", 4) == 0);

    if (isWebP) {
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        size_t fileSize = (size_t)f.tellg();
        f.seekg(0, std::ios::beg);
        std::vector<uint8_t> buf(fileSize);
        if (!f.read(reinterpret_cast<char*>(buf.data()), (std::streamsize)fileSize))
            return 0;

        int w, h;
        uint8_t* decoded = WebPDecodeRGBA(buf.data(), fileSize, &w, &h);
        if (!decoded) return 0;

        // Pre-multiply alpha for NanoVG
        for (int i = 0; i < w * h; i++) {
            uint8_t* p = decoded + i * 4;
            float a = p[3] / 255.0f;
            p[0] = (uint8_t)(p[0] * a);
            p[1] = (uint8_t)(p[1] * a);
            p[2] = (uint8_t)(p[2] * a);
        }

        int img = nvgCreateImageRGBA(vg, w, h, 0, decoded);
        WebPFree(decoded);
        return img;
    }

    // Not WebP — use NanoVG's built-in loader
    return nvgCreateImage(vg, path.c_str(), 0);
}
