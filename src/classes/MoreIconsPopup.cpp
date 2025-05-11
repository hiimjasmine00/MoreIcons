#include "MoreIconsPopup.hpp"
#include "EditIconPopup.hpp"
#include "IconViewPopup.hpp"
#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/ui/SimpleAxisLayout.hpp>

using namespace geode::prelude;

MoreIconsPopup* MoreIconsPopup::create() {
    auto ret = new MoreIconsPopup();
    if (ret->initAnchored(440.0f, 290.0f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MoreIconsPopup::setup() {
    setID("MoreIconsPopup");
    setTitle("More Icons", "goldFont.fnt", 0.7f, 17.0f);
    m_title->setID("more-icons-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    auto gamemodesNode = CCNode::create();
    gamemodesNode->setPosition({ 220.0f, 135.0f });
    gamemodesNode->setContentSize({ 420.0f, 245.0f });
    gamemodesNode->setAnchorPoint({ 0.5f, 0.5f });
    gamemodesNode->setLayout(RowLayout::create()->setGap(5.0f)->setGrowCrossAxis(true));
    gamemodesNode->setID("gamemodes-node");

    constexpr std::array gamemodes = {
        std::tuple("Icons", IconType::Cube, ccColor3B { 64, 227, 72 }),
        std::tuple("Ships", IconType::Ship, ccColor3B { 255, 0, 255 }),
        std::tuple("Balls", IconType::Ball, ccColor3B { 255, 90, 90 }),
        std::tuple("UFOs", IconType::Ufo, ccColor3B { 255, 165, 75 }),
        std::tuple("Waves", IconType::Wave, ccColor3B { 50, 200, 255 }),
        std::tuple("Robots", IconType::Robot, ccColor3B { 200, 200, 200 }),
        std::tuple("Spiders", IconType::Spider, ccColor3B { 150, 50, 255 }),
        std::tuple("Swings", IconType::Swing, ccColor3B { 255, 255, 0 }),
        std::tuple("Jetpacks", IconType::Jetpack, ccColor3B { 255, 150, 255 }),
        std::tuple("Trails", IconType::Special, ccColor3B { 74, 82, 225 })
    };

    auto gameManager = GameManager::get();
    for (int i = 0; i < 10; i++) {
        auto gamemodeMenu = CCMenu::create();
        gamemodeMenu->setPosition({ 0.0f, 0.0f });
        gamemodeMenu->setContentSize({ 70.0f, 120.0f });
        gamemodeMenu->ignoreAnchorPointForPosition(false);
        gamemodeMenu->setID(fmt::format("gamemode-menu-{}", i + 1));

        auto background = CCScale9Sprite::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
        background->setPosition({ 35.0f, 60.0f });
        background->setContentSize({ 70.0f, 120.0f });
        background->setOpacity(105);
        background->setID("background");
        gamemodeMenu->addChild(background);

        auto& [name, type, color] = gamemodes[i];

        auto icon = GJItemIcon::createBrowserItem(gameManager->iconTypeToUnlockType(type), 1);
        icon->setPosition({ 35.0f, 100.0f });
        icon->setID("item-icon");
        gamemodeMenu->addChild(icon);

        auto& severity = MoreIcons::severities[type];
        if (severity > Severity::Debug) {
            auto severityIcon = CCSprite::createWithSpriteFrameName(MoreIcons::severityFrames[severity]);
            severityIcon->setPosition({ 50.0f, 115.0f });
            severityIcon->setScale(0.6f);
            severityIcon->setID("severity-icon");
            gamemodeMenu->addChild(severityIcon);
        }

        auto label = CCLabelBMFont::create(name, "bigFont.fnt");
        label->setPosition({ 35.0f, 76.0f });
        label->limitLabelWidth(65.0f, 0.45f, 0.0f);
        label->setColor(color);
        label->setID("info-label");
        gamemodeMenu->addChild(label);

        auto vanillaCount = gameManager->countForType(type);
        auto customCount = MoreIconsAPI::getCount(type);
        auto logCount = std::ranges::count_if(MoreIcons::logs, [type](const LogData& log) { return log.type == type; });

        auto vanillaLabel = CCLabelBMFont::create(fmt::format("Vanilla: {}", vanillaCount).c_str(), "goldFont.fnt");
        vanillaLabel->limitLabelWidth(65.0f, 0.4f, 0.0f);
        auto vanillaButton = CCMenuItemExt::createSpriteExtra(vanillaLabel, [type](auto) {
            IconViewPopup::create(type, false)->show();
        });
        vanillaButton->setPosition({ 35.0f, 62.0f });
        vanillaButton->setID("vanilla-button");
        gamemodeMenu->addChild(vanillaButton);

        auto customLabel = CCLabelBMFont::create(fmt::format("Custom: {}", customCount).c_str(), "goldFont.fnt");
        customLabel->limitLabelWidth(65.0f, 0.4f, 0.0f);
        auto customButton = CCMenuItemExt::createSpriteExtra(customLabel, [type](auto) {
            IconViewPopup::create(type, true)->show();
        });
        customButton->setPosition({ 35.0f, 49.0f });
        customButton->setID("custom-button");
        gamemodeMenu->addChild(customButton);

        auto logLabel = CCLabelBMFont::create(fmt::format("Logs: {}", logCount).c_str(), "goldFont.fnt");
        logLabel->limitLabelWidth(65.0f, 0.4f, 0.0f);
        auto logButton = CCMenuItemExt::createSpriteExtra(logLabel, [](auto) {});
        logButton->setPosition({ 35.0f, 36.0f });
        logButton->setID("log-button");
        gamemodeMenu->addChild(logButton);

        auto addSprite = ButtonSprite::create("Add", 0.8f);
        addSprite->setScale(0.7f);
        auto addButton = CCMenuItemExt::createSpriteExtra(addSprite, [type](auto) {
            EditIconPopup::create(type, 0, "", false)->show();
        });
        addButton->setPosition({ 35.0f, 15.0f });
        addButton->setScale(1.0f);
        addButton->setID("add-button");
        gamemodeMenu->addChild(addButton);

        gamemodesNode->addChild(gamemodeMenu);
    }

    gamemodesNode->updateLayout();
    m_mainLayer->addChild(gamemodesNode);

    return true;
}
