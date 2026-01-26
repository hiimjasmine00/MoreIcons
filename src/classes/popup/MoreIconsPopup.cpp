#include "MoreIconsPopup.hpp"
#include "edit/EditIconPopup.hpp"
#include "edit/EditTrailPopup.hpp"
#include "log/LogLayer.hpp"
#include "view/IconViewPopup.hpp"
#include "../../MoreIcons.hpp"
#include "../../utils/Constants.hpp"
#include "../../utils/Get.hpp"
#include "../../utils/Log.hpp"
#include "../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Mod.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

MoreIconsPopup* MoreIconsPopup::create() {
    auto ret = new MoreIconsPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MoreIconsPopup::init() {
    if (!BasePopup::init(460.0f, 290.0f, "geode.loader/GE_square02.png")) return false;

    auto& metadata = Mod::get()->getMetadata();

    setID("MoreIconsPopup");
    setTitle(fmt::format("{} {}", metadata.getName(), metadata.getVersion().toNonVString()), "goldFont.fnt", 0.7f, 17.0f);
    m_title->setID("more-icons-title");

    m_gamemodesNode = CCNode::create();
    m_gamemodesNode->setPosition({ 230.0f, 135.0f });
    m_gamemodesNode->setContentSize({ 440.0f, 245.0f });
    m_gamemodesNode->setAnchorPoint({ 0.5f, 0.5f });
    m_gamemodesNode->setID("gamemodes-node");
    m_mainLayer->addChild(m_gamemodesNode);

    m_dual = MoreIcons::dualSelected();
    m_color1 = MoreIcons::vanillaColor1(m_dual);
    m_color2 = MoreIcons::vanillaColor2(m_dual);
    m_colorGlow = MoreIcons::vanillaColorGlow(m_dual);
    m_glow = MoreIcons::vanillaGlow(m_dual);

    createMenu(IconType::Cube);
    createMenu(IconType::Ship);
    createMenu(IconType::Ball);
    createMenu(IconType::Ufo);
    createMenu(IconType::Wave);
    createMenu(IconType::Robot);
    createMenu(IconType::Spider);
    createMenu(IconType::Swing);
    createMenu(IconType::Jetpack);
    createMenu(IconType::DeathEffect);
    createMenu(IconType::Special);
    createMenu(IconType::ShipFire);

    m_gamemodesNode->setLayout(RowLayout::create()->setGap(5.0f)->setGrowCrossAxis(true));

    auto trashSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    trashSprite->setScale(0.8f);
    auto trashButton = CCMenuItemSpriteExtra::create(trashSprite, this, menu_selector(MoreIconsPopup::onTrash));
    trashButton->setPosition({ 455.0f, 5.0f });
    trashButton->setID("trash-button");
    m_buttonMenu->addChild(trashButton);

    handleTouchPriority(this);

    return true;
}

void MoreIconsPopup::onTrash(CCObject* sender) {
    if (auto res = MoreIcons::createTrash()) file::openFolder(res.unwrap());
    else Notify::error(res.unwrapErr());
}

void MoreIconsPopup::createMenu(IconType type) {
    auto gamemodeMenu = CCMenu::create();
    gamemodeMenu->setPosition({ 0.0f, 0.0f });
    gamemodeMenu->setContentSize({ 70.0f, 120.0f });
    gamemodeMenu->ignoreAnchorPointForPosition(false);
    gamemodeMenu->setID(fmt::format("{}-menu", Constants::getPluralLowercase(type)));
    m_gamemodesNode->addChild(gamemodeMenu);

    auto background = CCScale9Sprite::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    background->setPosition({ 35.0f, 60.0f });
    background->setContentSize({ 70.0f, 120.0f });
    background->setOpacity(105);
    background->setID("background");
    gamemodeMenu->addChild(background);

    auto id = MoreIcons::vanillaIcon(type, m_dual);
    if (type <= IconType::Jetpack) {
        auto icon = SimplePlayer::create(1);
        icon->updatePlayerFrame(id, type);
        more_icons::updateSimplePlayer(icon, type, m_dual);
        icon->setColor(m_color1);
        icon->setSecondColor(m_color2);
        icon->enableCustomGlowColor(m_colorGlow);
        icon->m_hasGlowOutline = m_glow;
        icon->updateColors();
        icon->setPosition({ 35.0f, 100.0f });
        icon->setScale(0.9f);
        icon->setID("player-icon");
        gamemodeMenu->addChild(icon);
    }
    else if (type >= IconType::DeathEffect) {
        auto info = more_icons::activeIcon(type, m_dual);
        auto sprite = info ? MoreIcons::customIcon(info) : CCSprite::createWithSpriteFrameName(
            type == IconType::DeathEffect ? fmt::format("explosionIcon_{:02}_001.png", id).c_str() :
            type == IconType::Special ? fmt::format("player_special_{:02}_001.png", id).c_str() :
            type == IconType::ShipFire ? fmt::format("shipfireIcon_{:02}_001.png", id).c_str() : "cc_2x2_white_image"
        );
        sprite->setPosition({ 35.0f, 100.0f });
        sprite->setScale(0.9f);
        sprite->setID("player-icon");
        gamemodeMenu->addChild(sprite);
    }

    size_t logsSize = 0;
    if (auto it = Log::logs.find(type); it != Log::logs.end()) {
        auto& logs = it->second;
        logsSize = logs.size();
        auto severityIcon = CCSprite::createWithSpriteFrameName(Constants::getSeverityFrame(logs[0].severity));
        severityIcon->setPosition({ 48.5f, 113.5f });
        severityIcon->setScale(0.5f);
        severityIcon->setID("severity-icon");
        gamemodeMenu->addChild(severityIcon);
    }

    auto label = CCLabelBMFont::create(Constants::getPluralUppercase(type).data(), "bigFont.fnt");
    label->setPosition({ 35.0f, 76.0f });
    label->limitLabelWidth(65.0f, 0.45f, 0.0f);
    switch (type) {
        case IconType::Cube: label->setColor({ 64, 227, 72 }); break;
        case IconType::Ship: label->setColor({ 255, 0, 255 }); break;
        case IconType::Ball: label->setColor({ 255, 90, 90 }); break;
        case IconType::Ufo: label->setColor({ 255, 165, 75 }); break;
        case IconType::Wave: label->setColor({ 50, 200, 255 }); break;
        case IconType::Robot: label->setColor({ 200, 200, 200 }); break;
        case IconType::Spider: label->setColor({ 150, 50, 255 }); break;
        case IconType::Swing: label->setColor({ 255, 255, 0 }); break;
        case IconType::Jetpack: label->setColor({ 255, 150, 255 }); break;
        case IconType::DeathEffect: label->setColor({ 150, 255, 255 }); break;
        case IconType::Special: label->setColor({ 74, 82, 225 }); break;
        case IconType::ShipFire: label->setColor({ 96, 171, 239 }); break;
        default: label->setColor({ 255, 0, 0 }); break;
    };
    label->setID("info-label");
    gamemodeMenu->addChild(label);

    auto vanillaLabel = CCLabelBMFont::create(fmt::format("Vanilla: {}", Get::GameManager()->countForType(type)).c_str(), "goldFont.fnt");
    vanillaLabel->limitLabelWidth(65.0f, 0.4f, 0.0f);
    auto vanillaButton = CCMenuItemSpriteExtra::create(vanillaLabel, this, menu_selector(MoreIconsPopup::onVanilla));
    vanillaButton->setPosition({ 35.0f, 62.0f });
    vanillaButton->setTag((int)type);
    vanillaButton->setID("vanilla-button");
    gamemodeMenu->addChild(vanillaButton);

    auto customLabel = CCLabelBMFont::create(fmt::format("Custom: {}", more_icons::getIconCount(type)).c_str(), "goldFont.fnt");
    customLabel->limitLabelWidth(65.0f, 0.4f, 0.0f);
    auto customButton = CCMenuItemSpriteExtra::create(customLabel, this, menu_selector(MoreIconsPopup::onCustom));
    customButton->setPosition({ 35.0f, 49.0f });
    customButton->setTag((int)type);
    customButton->setID("custom-button");
    gamemodeMenu->addChild(customButton);

    auto logLabel = CCLabelBMFont::create(fmt::format("Logs: {}", logsSize).c_str(), "goldFont.fnt");
    logLabel->limitLabelWidth(65.0f, 0.4f, 0.0f);
    auto logButton = CCMenuItemSpriteExtra::create(logLabel, this, menu_selector(MoreIconsPopup::onLogs));
    logButton->setPosition({ 35.0f, 36.0f });
    logButton->setTag((int)type);
    logButton->setID("log-button");
    gamemodeMenu->addChild(logButton);

    auto addSprite = ButtonSprite::create("Add", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    addSprite->setScale(0.6f);
    auto addButton = CCMenuItemSpriteExtra::create(addSprite, this, menu_selector(MoreIconsPopup::onAdd));
    addButton->setPosition({ 24.0f, 15.0f });
    addButton->setTag((int)type);
    addButton->setID("add-button");
    gamemodeMenu->addChild(addButton);

    auto folderSprite = ButtonSprite::create(
        CCSprite::createWithSpriteFrameName("folderIcon_001.png"), 0, false, 0.0f, "GJ_button_05.png", 0.7f);
    folderSprite->setScale(0.45f);
    auto folderButton = CCMenuItemSpriteExtra::create(folderSprite, this, menu_selector(MoreIconsPopup::onFolder));
    folderButton->setPosition({ 54.0f, 15.0f });
    folderButton->setTag((int)type);
    folderButton->setID("folder-button");
    gamemodeMenu->addChild(folderButton);
}

void MoreIconsPopup::onVanilla(CCObject* sender) {
    IconViewPopup::create((IconType)sender->getTag(), false)->show();
}

void MoreIconsPopup::onCustom(CCObject* sender) {
    IconViewPopup::create((IconType)sender->getTag(), true)->show();
}

void MoreIconsPopup::onLogs(CCObject* sender) {
    LogLayer::create((IconType)sender->getTag())->show();
}

void MoreIconsPopup::onAdd(CCObject* sender) {
    auto type = (IconType)sender->getTag();
    if (type <= IconType::Jetpack) EditIconPopup::create(this, type)->show();
    else if (type == IconType::Special) EditTrailPopup::create(this)->show();
}

void MoreIconsPopup::onFolder(CCObject* sender) {
    file::openFolder(MoreIcons::getIconDir((IconType)sender->getTag()));
}
