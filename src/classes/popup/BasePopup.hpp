#pragma once
#include <Geode/ui/Popup.hpp>

class BasePopup : public geode::Popup {
protected:
    bool init(float width, float height, const char* background = "GJ_square01.png");
public:
    void close();
};
