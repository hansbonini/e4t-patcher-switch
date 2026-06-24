#pragma once

#include <borealis.hpp>

class MainFrame : public brls::AppletFrame
{
public:
    MainFrame();
    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;

private:
    int bgImage = 0;
};
