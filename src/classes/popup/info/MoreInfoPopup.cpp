#include "MoreInfoPopup.hpp"
#include "SpecialSettingsPopup.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/TextArea.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

MoreInfoPopup* MoreInfoPopup::create(IconInfo* info) {
    auto ret = new MoreInfoPopup();
    if (ret->initAnchored(300.0f, 230.0f, info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MoreInfoPopup::setup(IconInfo* info) {
    setID("MoreInfoPopup");
    setTitle(info->name.substr(info->name.find(':') + 1), "goldFont.fnt", 0.8f, 16.0f);
    m_title->setID("more-info-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");
    m_closeBtn->setVisible(false);

    auto descBackground = CCScale9Sprite::create("GJ_square01.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    descBackground->setPosition({ 150.0f, 93.0f });
    descBackground->setContentSize({ 260.0f, 90.0f });
    descBackground->setColor({ 0, 0, 0 });
    descBackground->setOpacity(60);
    descBackground->setID("description-bg");
    m_mainLayer->addChild(descBackground);

    auto hasPack = !info->packID.empty();
    auto gameManager = GameManager::get();
    auto unlockType = gameManager->iconTypeToUnlockType(info->type);

    auto customLabel = CCLabelBMFont::create("Custom", "goldFont.fnt");
    customLabel->setPosition({ 150.0f, 123.0f });
    customLabel->setScale(0.55f);
    customLabel->setID("custom-label");
    m_mainLayer->addChild(customLabel);

    constexpr std::array names = {
        "", "Cube", "Main Color", "Secondary Color", "Ship", "Ball", "UFO", "Wave",
        "Robot", "Spider", "Trail", "Death Effect", "Item", "Swing", "Jetpack", "Ship Fire"
    };

    auto descriptionArea = TextArea::create(fmt::format("This <cg>{}</c> is added by the <cl>More Icons</c> mod.",
        names[(int)unlockType]), "bigFont.fnt", 1.0f, 600.0f, { 0.5f, 1.0f }, 42.0f, false);
    descriptionArea->setPosition({ 150.0f, 91.0f });
    descriptionArea->setScale(0.4f);
    descriptionArea->setID("description-area");
    m_mainLayer->addChild(descriptionArea);

    if (hasPack) {
        auto packLabel = CCLabelBMFont::create(info->packName.c_str(), "goldFont.fnt");
        packLabel->setPosition({ 150.0f, 198.0f });
        packLabel->setScale(0.4f);
        packLabel->setID("pack-label");
        m_mainLayer->addChild(packLabel);
    }

    if (info->type <= IconType::Jetpack) {
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);

        auto itemIcon = GJItemIcon::createBrowserItem(unlockType, 1);
        itemIcon->setScale(1.25f - hasPack * 0.15f);

        auto player = static_cast<SimplePlayer*>(itemIcon->m_player);
        MoreIconsAPI::updateSimplePlayer(player, info->name, info->type);
        player->enableCustomGlowColor(gameManager->colorForIdx(dual ? sdi->getSavedValue("colorglow", 0) : gameManager->m_playerGlowColor));

        auto color1 = gameManager->colorForIdx(dual ? sdi->getSavedValue("color1", 0) : gameManager->m_playerColor);
        auto color2 = gameManager->colorForIdx(dual ? sdi->getSavedValue("color2", 0) : gameManager->m_playerColor2);
        auto glow = dual ? sdi->getSavedValue("glow", false) : gameManager->m_playerGlow;
        auto iconButton = CCMenuItemExt::createSpriteExtra(itemIcon, [this, color1, color2, glow](CCMenuItemSpriteExtra* sender) {
            m_toggled = !m_toggled;

            auto player = static_cast<SimplePlayer*>(static_cast<GJItemIcon*>(sender->getNormalImage())->m_player);
            player->m_firstLayer->setColor(m_toggled ? color1 : ccColor3B { 175, 175, 175 });
            player->m_secondLayer->setColor(m_toggled ? color2 : ccColor3B { 255, 255, 255 });
            player->m_hasGlowOutline = m_toggled && glow;
            player->updateColors();
        });
        iconButton->setPosition({ 150.0f, 171.0f - hasPack * 6.0f });
        iconButton->setID("icon-button");
        m_buttonMenu->addChild(iconButton);
    }
    else if (info->type == IconType::Special) {
        auto square = MoreIconsAPI::customTrail(info->textures[0]);
        square->setPosition({ 150.0f, 171.0f - hasPack * 6.0f });
        square->setScale(1.25f - hasPack * 0.15f);
        square->setID("trail-square");
        m_mainLayer->addChild(square);

        if (info->trailID == 0) {
            auto settingsButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_optionsBtn_001.png", 0.7f, [info](auto) {
                SpecialSettingsPopup::create(info)->show();
            });
            settingsButton->setPosition({ 25.0f, 25.0f });
            settingsButton->setID("settings-button");
            m_buttonMenu->addChild(settingsButton);
        }
    }

    auto okButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("OK"), [this](auto) {
        onClose(nullptr);
    });
    okButton->setPosition({ 150.0f, 25.0f });
    okButton->setID("ok-button");
    m_buttonMenu->addChild(okButton);

    return true;
}
