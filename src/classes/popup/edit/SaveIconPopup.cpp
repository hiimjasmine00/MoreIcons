#include "SaveIconPopup.hpp"
#include "EditIconPopup.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>

using namespace geode::prelude;

SaveIconPopup* SaveIconPopup::create(EditIconPopup* popup, IconType type, CCDictionary* definitions, CCDictionary* frames) {
    auto ret = new SaveIconPopup();
    if (ret->initAnchored(350.0f, 130.0f, popup, type, definitions, frames, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SaveIconPopup::setup(EditIconPopup* popup, IconType type, CCDictionary* definitions, CCDictionary* frames) {
    setID("SaveIconPopup");
    setTitle(fmt::format("Save {}", MoreIconsAPI::uppercase[(int)MoreIconsAPI::convertType(type)]));
    m_title->setID("save-icon-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_parentPopup = popup;
    m_iconType = type;
    m_definitions = definitions;
    m_frames = frames;

    m_nameInput = TextInput::create(300.0f, "Icon Name");
    m_nameInput->setPosition({ 175.0f, 70.0f });
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_nameInput->setMaxCharCount(100);
    m_nameInput->setID("name-input");
    m_mainLayer->addChild(m_nameInput);

    auto saveSprite = ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto saveButton = CCMenuItemExt::createSpriteExtra(saveSprite, [](auto) {});
    saveButton->setPosition({ 175.0f, 30.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}

void SaveIconPopup::onClose(CCObject* sender) {
    if (m_nameInput->getString().empty()) return Popup::onClose(sender);

    auto type = MoreIconsAPI::convertType(m_iconType);
    createQuickPopup(
        fmt::format("Exit {} Saver", MoreIconsAPI::uppercase[type]).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} saver</c>?", MoreIconsAPI::lowercase[type]),
        "No",
        "Yes",
        [this](auto, bool btn2) {
            if (btn2) Popup::onClose(m_closeBtn);
        }
    );
}
