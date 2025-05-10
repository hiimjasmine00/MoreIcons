#include "../api/MoreIconsAPI.hpp"
#include "MoreInfoPopup.hpp"
#include "SpecialSettingsPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/binding/ItemInfoPopup.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/TextArea.hpp>

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
    setTitle(info->name.substr(info->name.find(':') + 1).c_str(), "goldFont.fnt", 0.8f, 16.0f);
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

    auto descriptionArea = TextArea::create(fmt::format("This <cg>{}</c> is added by the <cl>More Icons</c> mod.",
        GEODE_ANDROID(std::string)(ItemInfoPopup::nameForUnlockType(1, unlockType))), "bigFont.fnt", 1.0f, 600.0f, { 0.5f, 1.0f }, 42.0f, false);
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
        auto itemIcon = GJItemIcon::createBrowserItem(unlockType, 1);
        itemIcon->setScale(1.25f - hasPack * 0.15f);

        auto player = static_cast<SimplePlayer*>(itemIcon->m_player);
        MoreIconsAPI::updateSimplePlayer(player, info->name, info->type);
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        player->m_glowColor = gameManager->colorForIdx(
            sdi && sdi->getSavedValue("2pselected", false) ? sdi->getSavedValue("colorglow", 0) : gameManager->m_playerGlowColor);
        player->m_hasCustomGlowColor = true;

        auto iconButton = CCMenuItemExt::createSpriteExtra(itemIcon, [](CCMenuItemSpriteExtra* sender) {
            auto toggled = static_cast<CCBool*>(sender->getUserObject("toggled"_spr));
            if (!toggled) return;

            toggled->m_bValue = !toggled->m_bValue;

            auto player = static_cast<SimplePlayer*>(static_cast<GJItemIcon*>(sender->getNormalImage())->m_player);
            auto gameManager = GameManager::get();
            auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
            auto dual = sdi && sdi->getSavedValue("2pselected", false);
            player->m_firstLayer->setColor(toggled->m_bValue ?
                gameManager->colorForIdx(dual ? sdi->getSavedValue("color1", 0) : gameManager->m_playerColor) : ccColor3B { 175, 175, 175 });
            player->m_secondLayer->setColor(toggled->m_bValue ?
                gameManager->colorForIdx(dual ? sdi->getSavedValue("color2", 0) : gameManager->m_playerColor2) : ccColor3B { 255, 255, 255 });
            player->m_hasGlowOutline = toggled->m_bValue &&
                (dual ? sdi->getSavedValue("glow", false) : gameManager->m_playerGlow);
            player->updateColors();
        });
        iconButton->setPosition({ 150.0f, 171.0f - hasPack * 6.0f });
        iconButton->setUserObject("toggled"_spr, CCBool::create(false));
        iconButton->setID("icon-button");
        m_buttonMenu->addChild(iconButton);
    }
    else if (info->type == IconType::Special) {
        auto square = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
        square->setColor({ 150, 150, 150 });
        square->setPosition({ 150.0f, 171.0f - hasPack * 6.0f });
        square->setScale(1.25f - hasPack * 0.15f);
        square->setID("trail-square");
        m_mainLayer->addChild(square);

        auto streak = CCSprite::create(info->textures[0].c_str());
        limitNodeHeight(streak, 27.0f, 999.0f, 0.001f);
        streak->setRotation(-90.0f);
        streak->setPosition(square->getContentSize() / 2);
        square->addChild(streak);

        if (info->trailID <= 0) {
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
