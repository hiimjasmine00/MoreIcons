#include "MoreIconsPopup.hpp"
#include "EditIconPopup.hpp"
#include "IconViewPopup.hpp"
#include "LogLayer.hpp"
#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGarageLayer.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

MoreIconsPopup* MoreIconsPopup::create() {
    auto ret = new MoreIconsPopup();
    if (ret->initAnchored(440.0f, 290.0f, "geode.loader/GE_square02.png")) {
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
        std::tuple("Icons", IconType::Cube, ccColor3B { 64, 227, 72 }, "icon"),
        std::tuple("Ships", IconType::Ship, ccColor3B { 255, 0, 255 }, "ship"),
        std::tuple("Balls", IconType::Ball, ccColor3B { 255, 90, 90 }, "ball"),
        std::tuple("UFOs", IconType::Ufo, ccColor3B { 255, 165, 75 }, "ufo"),
        std::tuple("Waves", IconType::Wave, ccColor3B { 50, 200, 255 }, "wave"),
        std::tuple("Robots", IconType::Robot, ccColor3B { 200, 200, 200 }, "robot"),
        std::tuple("Spiders", IconType::Spider, ccColor3B { 150, 50, 255 }, "spider"),
        std::tuple("Swings", IconType::Swing, ccColor3B { 255, 255, 0 }, "swing"),
        std::tuple("Jetpacks", IconType::Jetpack, ccColor3B { 255, 150, 255 }, "jetpack"),
        std::tuple("Trails", IconType::Special, ccColor3B { 74, 82, 225 }, "trail")
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

        auto& [name, type, color, directory] = gamemodes[i];

        auto unlock = gameManager->iconTypeToUnlockType(type);
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        constexpr std::array types = {
            "", "cube", "color1", "color2", "ship", "roll", "bird", "dart",
            "robot", "spider", "trail", "death", "", "swing", "jetpack", "shiptrail"
        };

        auto icon = GJItemIcon::createBrowserItem(unlock, dual ? sdi->getSavedValue<int>(types[(int)unlock]) : gameManager->activeIconForType(type));
        if (type <= IconType::Jetpack) queueInMainThread([icon = Ref(icon), type, dual, gameManager, sdi] {
            auto player = static_cast<SimplePlayer*>(icon->m_player);
            MoreIconsAPI::updateSimplePlayer(player, type, dual);
            player->setColor(gameManager->colorForIdx(dual ? sdi->getSavedValue<int>("color1") : gameManager->m_playerColor));
            player->setSecondColor(gameManager->colorForIdx(dual ? sdi->getSavedValue<int>("color2") : gameManager->m_playerColor2));
            player->enableCustomGlowColor(gameManager->colorForIdx(dual ? sdi->getSavedValue<int>("colorglow") : gameManager->m_playerGlowColor));
            player->m_hasGlowOutline = dual ? sdi->getSavedValue<bool>("glow") : gameManager->m_playerGlow;
            player->updateColors();
        });
        icon->setPosition({ 35.0f, 100.0f });
        icon->setScale(0.9f);
        icon->setID("item-icon");
        gamemodeMenu->addChild(icon);

        auto& severity = MoreIcons::severities[type];
        if (severity > Severity::Debug) {
            auto severityIcon = CCSprite::createWithSpriteFrameName(MoreIcons::severityFrames[severity]);
            severityIcon->setPosition({ 48.5f, 113.5f });
            severityIcon->setScale(0.5f);
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
        auto logButton = CCMenuItemExt::createSpriteExtra(logLabel, [type](auto) {
            LogLayer::create(type)->show();
        });
        logButton->setPosition({ 35.0f, 36.0f });
        logButton->setID("log-button");
        gamemodeMenu->addChild(logButton);

        auto addSprite = ButtonSprite::create("Add", 0, false, "goldFont.fnt", "GJ_button_05.png", 0.0f, 0.8f);
        addSprite->setScale(0.6f);
        auto addButton = CCMenuItemExt::createSpriteExtra(addSprite, [type](auto) {
            EditIconPopup::create(type, 0, "", false)->show();
        });
        addButton->setPosition({ 24.0f, 15.0f });
        addButton->setID("add-button");
        gamemodeMenu->addChild(addButton);

        auto folderSprite = ButtonSprite::create(CCSprite::createWithSpriteFrameName("folderIcon_001.png"), 0, false, 0.0f, "GJ_button_05.png", 0.7f);
        folderSprite->setScale(0.45f);
        auto folderButton = CCMenuItemExt::createSpriteExtra(folderSprite, [directory](auto) {
            file::openFolder(Mod::get()->getConfigDir() / directory);
        });
        folderButton->setPosition({ 54.0f, 15.0f });
        folderButton->setID("folder-button");
        gamemodeMenu->addChild(folderButton);

        gamemodesNode->addChild(gamemodeMenu);
    }

    gamemodesNode->updateLayout();
    m_mainLayer->addChild(gamemodesNode);

    auto trashButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_trashBtn_001.png", 0.8f, [](auto) {
        auto trashDir = Mod::get()->getConfigDir() / "trash";
        std::error_code code;
        auto exists = std::filesystem::exists(trashDir, code);
        if (!exists) exists = std::filesystem::create_directory(trashDir, code);
        if (!exists) return Notification::create("Failed to create trash directory", NotificationIcon::Error)->show();
        file::openFolder(trashDir);
    });
    trashButton->setPosition({ 435.0f, 5.0f });
    trashButton->setID("trash-button");
    m_buttonMenu->addChild(trashButton);

    auto reloadSprite = ButtonSprite::create("Reload Textures", 0, false, "goldFont.fnt", "GJ_button_05.png", 0.0f, 1.0f);
    reloadSprite->setScale(0.7f);
    m_reloadButton = CCMenuItemExt::createSpriteExtra(reloadSprite, [](auto) {
        reloadTextures([] {
            return GJGarageLayer::node();
        });
    });
    m_reloadButton->setPosition({ 220.0f, 0.0f });
    m_reloadButton->setVisible(MoreIcons::showReload);
    m_reloadButton->setID("reload-button");
    m_buttonMenu->addChild(m_reloadButton);

    return true;
}
