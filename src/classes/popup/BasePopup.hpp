#pragma once
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/ui/Popup.hpp>

class BasePopup : public geode::Popup {
protected:
    bool init(
        float width, float height, const char* background = "GJ_square01.png", geode::CircleBaseColor closeColor = geode::CircleBaseColor::Green
    );
public:
    void close();
};
