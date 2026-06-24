#pragma once

#include <borealis.hpp>
#include <json.hpp>

#include "constants.hpp"

class ListExtraTab : public brls::ScrollView
{
private:
    nlohmann::ordered_json nxlinks;
    contentType type;
    brls::BoxLayout* contentView = nullptr;
    brls::BoxLayout* currentRow = nullptr;
    int itemsInCurrentRow = 0;

    void buildGrid(contentType type);
    void addItemToGrid(brls::View* item);
    void finishCurrentRow();
    std::string downloadImage(const std::string& imageUrl);
    void displayNotFound();
    void noItemsToDisplay();
    void setDescription();

public:
    ListExtraTab(const contentType type, const nlohmann::ordered_json& nxlinks = nlohmann::ordered_json::object());
};
