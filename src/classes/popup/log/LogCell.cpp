#include "LogCell.hpp"
#include "../../../utils/Constants.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

LogCell* LogCell::create(std::string_view name, std::string_view message, Severity severity, bool light) {
    auto ret = new LogCell();
    if (ret->init(name, message, severity, light)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LogCell::init(std::string_view name, std::string_view message, Severity severity, bool light) {
    if (!CCLayer::init()) return false;

    setID(fmt::format("log-cell-{}", name));
    ignoreAnchorPointForPosition(false);
    setContentSize({ 400.0f, 30.0f });

    m_name = name.data();

    auto bg = CCLayerColor::create({ 0, 0, 0, 255 }, 400.0f, 70.0f);
    bg->setPosition({ 200.0f, 15.0f });
    bg->setContentSize({ 400.0f, 30.0f });
    bg->ignoreAnchorPointForPosition(false);
    bg->setColor(light ? ccColor3B { 50, 50, 50 } : ccColor3B { 40, 40, 40 });
    bg->setID("background");
    addChild(bg, -1);

    auto infoIcon = CCSprite::createWithSpriteFrameName(Constants::getSeverityFrame(severity));
    infoIcon->setPosition({ 15.0f, 15.0f });
    infoIcon->setScale(0.8f);
    infoIcon->setID("info-icon");
    addChild(infoIcon);

    auto nameLabel = CCLabelBMFont::create(m_name, "bigFont.fnt");
    nameLabel->setPosition({ 35.0f, 15.0f });
    nameLabel->setAnchorPoint({ 0.0f, 0.5f });
    nameLabel->limitLabelWidth(300.0f, 0.4f, 0.0f);
    nameLabel->setID("name-label");
    addChild(nameLabel);

    switch (severity) {
        case Severity::Debug: m_message = fmt::format("<cg>DEBUG:</c> {}", message); break;
        case Severity::Info: m_message = fmt::format("<cj>INFO:</c> {}", message); break;
        case Severity::Warning: m_message = fmt::format("<cy>WARNING:</c> {}", message); break;
        case Severity::Error: m_message = fmt::format("<cr>ERROR:</c> {}", message); break;
        default: m_message = message; break;
    }

    auto buttonMenu = CCMenu::create();
    buttonMenu->setPosition({ 370.0f, 15.0f });
    buttonMenu->setID("button-menu");
    addChild(buttonMenu);

    auto viewSprite = ButtonSprite::create("View", "bigFont.fnt", "GJ_button_05.png", 0.8f);
    viewSprite->setScale(0.5f);
    auto viewButton = CCMenuItemSpriteExtra::create(viewSprite, this, menu_selector(LogCell::onClick));
    viewButton->setID("view-button");
    buttonMenu->addChild(viewButton);

    return true;
}

void LogCell::onClick(CCObject* sender) {
    FLAlertLayer::create(m_name, m_message, "OK")->show();
}

void LogCell::draw() {
    ccDrawColor4B(0, 0, 0, 75);
    glLineWidth(2.0f);
    ccDrawLine({ 0.0f, 0.0f }, { 400.0f, 0.0f });
    ccDrawLine({ 0.0f, 30.0f }, { 400.0f, 30.0f });
}
