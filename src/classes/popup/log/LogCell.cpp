#include "LogCell.hpp"
#include "../../../MoreIcons.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

LogCell* LogCell::create(const char* name, std::string_view message, int severity, int index) {
    auto ret = new LogCell();
    if (ret->init(name, message, severity, index)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LogCell::init(const char* name, std::string_view message, int severity, int index) {
    if (!CCLayer::init()) return false;

    setID(fmt::format("log-cell-{}", index + 1));
    ignoreAnchorPointForPosition(false);
    setContentSize({ 400.0f, 30.0f });

    auto bg = CCLayerColor::create({ 0, 0, 0, 255 }, 400.0f, 70.0f);
    bg->setPosition({ 200.0f, 15.0f });
    bg->setContentSize({ 400.0f, 30.0f });
    bg->ignoreAnchorPointForPosition(false);
    bg->setColor(index % 2 == 0 ? ccColor3B { 50, 50, 50 } : ccColor3B { 40, 40, 40 });
    bg->setID("background");
    addChild(bg, -1);

    auto infoIcon = CCSprite::createWithSpriteFrameName(MoreIcons::severityFrames[severity]);
    infoIcon->setPosition({ 15.0f, 15.0f });
    infoIcon->setScale(0.8f);
    infoIcon->setID("info-icon");
    addChild(infoIcon);

    auto nameLabel = CCLabelBMFont::create(name, "bigFont.fnt");
    nameLabel->setPosition({ 35.0f, 15.0f });
    nameLabel->setAnchorPoint({ 0.0f, 0.5f });
    nameLabel->limitLabelWidth(300.0f, 0.4f, 0.0f);
    nameLabel->setID("name-label");
    addChild(nameLabel);

    constexpr std::array severities = { "<cg>DEBUG:</c> ", "<cj>INFO:</c> ", "<cy>WARNING:</c> ", "<cr>ERROR:</c> " };

    auto viewSprite = ButtonSprite::create("View", "bigFont.fnt", "GJ_button_05.png", 0.8f);
    viewSprite->setScale(0.5f);
    auto viewButton = CCMenuItemExt::createSpriteExtra(viewSprite, [name, message = fmt::format("{}{}", severities[severity], message)](auto) {
        FLAlertLayer::create(name, message, "OK")->show();
    });
    viewButton->setID("view-button");

    auto buttonMenu = CCMenu::createWithItem(viewButton);
    buttonMenu->setPosition({ 370.0f, 15.0f });
    buttonMenu->setID("button-menu");
    addChild(buttonMenu);

    return true;
}

void LogCell::draw() {
    ccDrawColor4B(0, 0, 0, 75);
    glLineWidth(2.0f);
    ccDrawLine({ 0.0f, 0.0f }, { 400.0f, 0.0f });
    ccDrawLine({ 0.0f, 30.0f }, { 400.0f, 30.0f });
}
