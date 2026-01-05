#include "BasePopup.hpp"

using namespace geode::prelude;

bool BasePopup::init(float width, float height, const char* background) {
    if (!Popup::initAnchored(width, height, background, { 0.0f, 0.0f, 80.0f, 80.0f })) return false;

    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    return true;
}

bool BasePopup::setup() {
    return true;
}

void BasePopup::close() {
    Popup::onClose(nullptr);
}

void BasePopup::keyBackClicked() {
    onClose(nullptr);
}

void BasePopup::keyDown(enumKeyCodes key) {
    return FLAlertLayer::keyDown(key);
}
