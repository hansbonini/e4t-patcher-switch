#pragma once

#include <borealis.hpp>

#include "constants.hpp"

class GridItem : public brls::View
{
public:
    GridItem(std::string title, std::string size, bool installed, std::string url, contentType type,
             std::vector<std::string> itemFolders, bool bInstalled,
             std::string imagePath = "", int imgW = 0, int imgH = 0);

    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;
    void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash) override;
    void frame(brls::FrameContext* ctx) override;
    brls::View* getDefaultFocus() override;

    bool isHighlightBackgroundEnabled() override { return false; }

    void getHighlightMetrics(brls::Style* style, float* cornerRadius) override
    {
        *cornerRadius = 10.0f;
    }

    void getHighlightInsets(unsigned* top, unsigned* right, unsigned* bottom, unsigned* left) override
    {
        *top    = 4;
        *right  = 4;
        *bottom = 4;
        *left   = 4;
    }

    brls::GenericEvent* getClickEvent() { return &clickEvent; }

    static int loadImage(NVGcontext* vg, const std::string& path);

    // Accessors for click handler
    const std::string& getTitle() const { return title; }
    const std::string& getUrl() const { return url; }
    const std::vector<std::string>& getItemFolders() const { return itemFolders; }
    bool getInstalled() const { return bInstalled; }

    static constexpr int CARD_WIDTH = 276;
    static constexpr int DEFAULT_CARD_HEIGHT = 48;
    static constexpr int MAX_CARD_HEIGHT = 360;
    static constexpr int LABEL_HEIGHT = 20;
    static constexpr int TOTAL_PADDING = 6;

    int getCardHeight() const { return cardHeight; }
    int getTotalHeight() const { return cardHeight + LABEL_HEIGHT + TOTAL_PADDING; }
    void setCardHeight(int h) { cardHeight = h; this->setHeight(getTotalHeight()); }

private:
    std::string title;
    std::string size;
    bool installed;
    std::string url;
    contentType type;
    std::vector<std::string> itemFolders;
    bool bInstalled;
    brls::GenericEvent clickEvent;

    // Image data
    std::string imagePath;
    int imageHandle = 0;
    int imgW = 0;
    int imgH = 0;
    int cardHeight = DEFAULT_CARD_HEIGHT;
    bool imageLoaded = false;
};
