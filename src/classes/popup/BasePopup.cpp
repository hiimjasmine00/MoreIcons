#include "BasePopup.hpp"

using namespace geode::prelude;

bool BasePopup::init(float width, float height, const char* background) {
    if (!Popup::init(width, height, background, { 0.0f, 0.0f, 80.0f, 80.0f })) return false;

    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    return true;
}

void BasePopup::close() {
    Popup::onClose(nullptr);
}
