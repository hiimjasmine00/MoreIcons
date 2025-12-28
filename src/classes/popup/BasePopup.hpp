#pragma once
#include <Geode/ui/Popup.hpp>

class BasePopup : public geode::Popup<> {
protected:
    bool init(float width, float height, const char* background = "GJ_square01.png");

    bool setup() override;
public:
    void close();
    void keyBackClicked() override;
    void keyDown(cocos2d::enumKeyCodes key) override;
};
