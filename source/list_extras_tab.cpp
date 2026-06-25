#include "list_extra_tab.hpp"

#include <filesystem>
#include <fstream>
#include <string>

#include "confirm_page.hpp"
#include "fs.hpp"
#include "grid_item.hpp"
#include "list_download_tab_confirmation.hpp"
#include "download.hpp"
#include "utils.hpp"
#include "worker_page.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

ListExtraTab::ListExtraTab(const contentType type, const nlohmann::ordered_json& nxlinks)
    : brls::ScrollView(), type(type), nxlinks(nxlinks)
{
    contentView = new brls::BoxLayout(brls::BoxLayoutOrientation::VERTICAL);
    contentView->setResize(true);
    this->setContentView(contentView);

    this->setDescription();
    this->buildGrid(type);
}

void ListExtraTab::addItemToGrid(brls::View* item)
{
    GridItem* gi = dynamic_cast<GridItem*>(item);
    int itemHeight = gi ? gi->getTotalHeight() : GridItem::DEFAULT_CARD_HEIGHT + GridItem::LABEL_HEIGHT + GridItem::TOTAL_PADDING;

    if (itemsInCurrentRow == 0) {
        currentRow = new brls::BoxLayout(brls::BoxLayoutOrientation::HORIZONTAL);
        currentRow->setResize(true);
        currentRow->setHeight(itemHeight);
        itemsInCurrentRow = 0;
    } else {
        // Add gap between cards in the same row
        brls::BoxLayout* gap = new brls::BoxLayout(brls::BoxLayoutOrientation::HORIZONTAL);
        gap->setWidth(20);
        gap->setHeight(1);
        currentRow->addView(gap);
    }

    currentRow->addView(item);
    itemsInCurrentRow++;

    if (itemsInCurrentRow >= 4) {
        this->finishCurrentRow();
    }
}

void ListExtraTab::finishCurrentRow()
{
    if (currentRow != nullptr) {
        contentView->addView(currentRow);
        brls::BoxLayout* vgap = new brls::BoxLayout(brls::BoxLayoutOrientation::HORIZONTAL);
        vgap->setHeight(16);
        contentView->addView(vgap);
        currentRow = nullptr;
        itemsInCurrentRow = 0;
    }
}

std::string ListExtraTab::downloadImage(const std::string& imageUrl)
{
    if (imageUrl.empty()) return "";

    // Extract filename from URL
    size_t lastSlash = imageUrl.find_last_of('/');
    std::string filename = (lastSlash != std::string::npos) ? imageUrl.substr(lastSlash + 1) : "image.jpg";

    // Remove query parameters from filename
    size_t qmark = filename.find('?');
    if (qmark != std::string::npos) filename = filename.substr(0, qmark);

    if (filename.empty()) filename = "image.jpg";

    std::string cacheDir = fmt::format(IMAGE_CACHE_PATH, BASE_FOLDER_NAME);
    std::string localPath = cacheDir + filename;

    // Download if not already cached
    if (!std::filesystem::exists(localPath)) {
        fs::createTree(cacheDir);
        download::downloadFile(imageUrl, localPath, OFF);
    }

    return localPath;
}

void ListExtraTab::buildGrid(contentType type)
{
    int counter = 0;
    std::vector<GridItem*> gridItems;

    std::string sInstalledFile;
    int sInstalledSize = -1;
    bool bInstalled;

    std::string path = util::getContentsPath();
    std::vector<std::string> itemFolders;

    const nlohmann::ordered_json jsonExtras = util::getValueFromKey(this->nxlinks, contentTypeNames[(int)type].data());
    if (jsonExtras.size()) {
        // First pass: create all GridItems, calculate individual heights
        for (auto it = jsonExtras.begin(); it != jsonExtras.end(); ++it)
        {
            bool enabled = (*it)["enabled"].get<bool>();

            if (enabled) {
                sInstalledFile.clear();
                sInstalledSize = -1;
                bInstalled = false;

                const std::string title = it.key();
                const std::string url = (*it)["link"].get<std::string>();
                const std::string size = (*it)["size"].get<std::string>();
                itemFolders.clear();

                // Download image
                std::string imagePath;
                int imgW = 0, imgH = 0;
                if ((*it).contains("image_link")) {
                    imagePath = this->downloadImage((*it)["image_link"].get<std::string>());
                    if (!imagePath.empty()) {
                        util::getImageDimensions(imagePath, imgW, imgH);
                    }
                }

                if ((*it)["installed"].contains("hash_file"))
                {
                    sInstalledFile = path + (*it)["installed"]["hash_file"].get<std::string>();
                    sInstalledSize = (*it)["installed"]["hash_size"].get<int>();

                    if (std::filesystem::exists(sInstalledFile))
                    {
                        FILE *p_file = NULL;
                        p_file = fopen(sInstalledFile.c_str(), "rb");
                        fseek(p_file, 0, SEEK_END);
                        int fsize = ftell(p_file);
                        fclose(p_file);

                        if (fsize == sInstalledSize)
                            bInstalled = true;
                        else
                            bInstalled = false;
                    }
                }

                if ((*it)["installed"].contains("uninstall"))
                {
                    for (auto it2 = (*it)["installed"]["uninstall"].begin(); it2 != (*it)["installed"]["uninstall"].end(); ++it2)
                    {
                        const std::string tmp = it2.value();
                        itemFolders.push_back(tmp);

                        if (sInstalledSize < 0)
                            bInstalled = std::filesystem::exists(path + tmp);
                    }
                }

                GridItem* item = new GridItem(title, size, bInstalled, url, type, itemFolders, bInstalled, imagePath, imgW, imgH);
                gridItems.push_back(item);
                counter++;
            }
        }

        // Normalize all card heights to the tallest one
        if (!gridItems.empty()) {
            int maxHeight = 0;
            for (auto* gi : gridItems) {
                int h = gi->getCardHeight();
                if (h > maxHeight) maxHeight = h;
            }
            for (auto* gi : gridItems) {
                gi->setCardHeight(maxHeight);
            }
        }

    // Second pass: subscribe click events and add to grid
    for (auto* item : gridItems)
    {
        const std::string text("menus/common/download"_i18n + std::string(item->getTitle()));

        item->getClickEvent()->subscribe([this, type, text, item](brls::View* view) {
            brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();

            if (item->getInstalled())
            {
                stagedFrame->setTitle("menus/common/deleting"_i18n);
                stagedFrame->addStage(new ListDownloadConfirmationPage(stagedFrame, DialogType::warning, "menus/main/translation_exists_warning"_i18n, "", "", false));
                stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/deleting"_i18n, [this, type, item]() { util::doDelete(item->getItemFolders()); }));
            }
            else
            {
                stagedFrame->setTitle(fmt::format("menus/main/getting"_i18n, contentTypeFullNames[(int)type].data()));
                stagedFrame->addStage(new ConfirmPage(stagedFrame, text));
                stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/downloading"_i18n, [this, type, item]() { util::downloadArchive(item->getUrl(), type); }));
                stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/extracting"_i18n, [this, type]() { util::extractArchive(type); }));
            }

            stagedFrame->addStage(new ConfirmPage(stagedFrame, "menus/common/all_done"_i18n, true));
            brls::Application::pushView(stagedFrame);
        });

        this->addItemToGrid(item);
    }

    this->finishCurrentRow();

    if (counter <= 0)
        this->noItemsToDisplay();
}
    else {
        this->displayNotFound();
    }
}

void ListExtraTab::displayNotFound()
{
    brls::Label* notFound = new brls::Label(
        brls::LabelStyle::SMALL,
        "menus/main/links_not_found"_i18n,
        true);
    notFound->setHorizontalAlign(NVG_ALIGN_CENTER);
    contentView->addView(notFound);
}

void ListExtraTab::noItemsToDisplay()
{
    brls::Label* notFound = new brls::Label(
        brls::LabelStyle::SMALL,
        "menus/main/no_items_to_display"_i18n,
        true);
    notFound->setHorizontalAlign(NVG_ALIGN_CENTER);
    contentView->addView(notFound);
}

void ListExtraTab::setDescription()
{
    // Top spacer
    brls::BoxLayout* topPad = new brls::BoxLayout(brls::BoxLayoutOrientation::HORIZONTAL);
    topPad->setHeight(12);
    contentView->addView(topPad);

    brls::Label* description = new brls::Label(brls::LabelStyle::DESCRIPTION, "Selecione uma tradu\u00e7\u00e3o para instalar", true);
    contentView->addView(description);

    // Spacer between description and grid
    brls::BoxLayout* spacer = new brls::BoxLayout(brls::BoxLayoutOrientation::HORIZONTAL);
    spacer->setHeight(16);
    contentView->addView(spacer);
}
