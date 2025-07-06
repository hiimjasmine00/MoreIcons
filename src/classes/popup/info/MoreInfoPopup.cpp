#include "MoreInfoPopup.hpp"
#include "IconNamePopup.hpp"
#include "SpecialSettingsPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/TextArea.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Notification.hpp>

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

template <typename... T>
void notify(NotificationIcon icon, fmt::format_string<T...> message, T&&... args) {
    Notification::create(fmt::format(message, std::forward<T>(args)...), icon)->show();
}

Result<> renameFile(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) {
    auto parent = newPath.parent_path();
    auto filename = newPath.filename();
    for (int i = 1; MoreIcons::doesExist(parent / filename); i++) {
        filename = fmt::format("{} ({}){}", newPath.stem(), i, newPath.extension());
    }
    std::error_code code;
    std::filesystem::rename(oldPath, parent / filename, code);
    if (code) return Err(code.message());
    else return Ok();
}

Result<> copyVanillaFile(const std::filesystem::path& src, const std::filesystem::path& dest, bool uhd) {
    #ifdef GEODE_IS_MOBILE
    auto fullSrc = (uhd ? dirs::getModConfigDir() / "weebify.high-graphics-android" / GEODE_STR(GEODE_GD_VERSION) : dirs::getResourcesDir()) / src;
    #else
    auto fullSrc = dirs::getResourcesDir() / src;
    #endif
    #ifdef GEODE_IS_ANDROID
    if (!uhd) {
        auto size = 0ul;
        auto data = CCFileUtils::get()->getFileData(fullSrc.c_str(), "rb", &size);
        if (!data) return Err("Failed to read file");

        std::vector vec(data, data + size);
        delete[] data;

        return file::writeBinary(dest, vec);
    }
    #endif
    std::error_code code;
    std::filesystem::copy_file(fullSrc, dest, code);
    if (code) return Err(code.message());
    else return Ok();
}

constexpr std::array folders = { "icon", "ship", "ball", "ufo", "wave", "robot", "spider", "swing", "jetpack" };

void MoreInfoPopup::moveIcon(const std::filesystem::path& directory, bool trash) {
    std::filesystem::path texturePath = m_info->textures[0];
    auto parentDir = texturePath.parent_path();

    if (m_info->type == IconType::Special) {
        auto filename = texturePath.filename();
        if (GEODE_UNWRAP_IF_ERR(err, renameFile(texturePath, directory / filename)))
            return notify(NotificationIcon::Error, "Failed to {} {}: {}.", trash ? "trash" : "move", filename, err);

        if (trash) {
            auto jsonName = filename.stem() + ".json";
            auto jsonPath = parentDir / jsonName;
            if (MoreIcons::doesExist(jsonPath)) {
                if (GEODE_UNWRAP_IF_ERR(err, renameFile(jsonPath, directory / jsonName)))
                    return notify(NotificationIcon::Error, "Failed to trash {}: {}.", jsonName, err);
            }
        }
    }
    else if (m_info->type <= IconType::Jetpack) {
        auto& shortName = m_info->shortName;
        std::vector<std::filesystem::path> files;

        auto uhdPng = parentDir / (shortName + "-uhd.png");
        if (MoreIcons::doesExist(uhdPng)) {
            files.push_back(uhdPng);
            auto filename = shortName + "-uhd.plist";
            auto plist = parentDir / filename;
            if (MoreIcons::doesExist(plist)) files.push_back(plist);
            else if (!trash) {
                if (GEODE_UNWRAP_IF_ERR(err, copyVanillaFile("icons/" + filename, directory / filename, true)))
                    return notify(NotificationIcon::Error, "Failed to copy {}: {}", filename, err);
            }
        }

        auto hdPng = parentDir / (shortName + "-hd.png");
        if (MoreIcons::doesExist(hdPng)) {
            files.push_back(hdPng);
            auto filename = shortName + "-hd.plist";
            auto plist = parentDir / filename;
            if (MoreIcons::doesExist(plist)) files.push_back(plist);
            else if (!trash) {
                if (GEODE_UNWRAP_IF_ERR(err, copyVanillaFile("icons/" + filename, directory / filename, false)))
                    return notify(NotificationIcon::Error, "Failed to copy {}: {}", filename, err);
            }
        }

        auto png = parentDir / (shortName + ".png");
        if (MoreIcons::doesExist(png)) {
            files.push_back(png);
            auto filename = shortName + ".plist";
            auto plist = parentDir / filename;
            if (MoreIcons::doesExist(plist)) files.push_back(plist);
            else if (!trash) {
                if (GEODE_UNWRAP_IF_ERR(err, copyVanillaFile("icons/" + filename, directory / filename, false)))
                    return notify(NotificationIcon::Error, "Failed to copy {}: {}", filename, err);
            }
        }

        std::error_code code;
        for (int i = 0; i < files.size(); i++) {
            auto& file = files[i];
            auto filename = file.filename();
            if (GEODE_UNWRAP_IF_ERR(err, renameFile(file, directory / filename))) {
                for (int j = 0; j < i; j++) {
                    auto& file2 = files[j];
                    std::filesystem::rename(directory / file2.filename(), file2, code);
                }
                return notify(NotificationIcon::Error, "Failed to {} {}: {}.", trash ? "trash" : "move", filename, err);
            }
        }
    }

    auto name = m_info->shortName;
    onClose(nullptr);
    if (trash) MoreIconsAPI::removeIcon(m_info);
    else MoreIconsAPI::moveIcon(m_info, directory);
    notify(NotificationIcon::Success, "{} {}ed!", name, trash ? "trash" : "convert");
    MoreIcons::updateGarage();
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

    m_info = info;

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

    CCMenuItemSpriteExtra* operationButton = nullptr;

    if (info->vanilla && !info->zipped) {
        operationButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.7f, [this, unlockType](auto) {
            auto lower = MoreIcons::lowercase[(int)unlockType];
            createQuickPopup(
                fmt::format("Convert {}", MoreIcons::uppercase[(int)unlockType]).c_str(),
                fmt::format("Are you sure you want to <cy>convert</c> this <cg>{}</c> into a <cl>More Icons</c> <cg>{}</c>?", lower, lower),
                "No",
                "Yes",
                [this](auto, bool btn2) {
                    if (!btn2) return;

                    auto parent = std::filesystem::path(m_info->textures[0]).parent_path();
                    auto type = m_info->type;
                    auto dir = type <= IconType::Jetpack
                        ? parent.parent_path() / "config" / GEODE_MOD_ID / folders[(int)type]
                        : parent / "config" / GEODE_MOD_ID / "trail";
                    std::error_code code;
                    auto exists = MoreIcons::doesExist(dir);
                    if (!exists) exists = std::filesystem::create_directories(dir, code);
                    if (!exists) return notify(NotificationIcon::Error, "Failed to create directory: {}", code.message());
                    moveIcon(dir, false);
                }
            );
        });
        operationButton->setID("move-button");
    }
    else if (!info->zipped) {
        m_title->setPosition({ 140.0f, m_size.height - 16.0f });

        auto editButton = CCMenuItemExt::createSpriteExtraWithFilename("MI_pencil_001.png"_spr, 0.7f, [info](auto) {
            IconNamePopup::create(info)->show();
        });
        editButton->setPosition(m_title->getPosition() + CCPoint { m_title->getScaledContentWidth() / 2.0f + 10.0f, 0.0f });
        editButton->setID("edit-button");
        m_buttonMenu->addChild(editButton);

        operationButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_trashBtn_001.png", 0.8f, [this, unlockType](auto) {
            createQuickPopup(
                fmt::format("Trash {}", MoreIcons::uppercase[(int)unlockType]).c_str(),
                fmt::format("Are you sure you want to <cr>trash</c> this <cg>{}</c>?", MoreIcons::lowercase[(int)unlockType]),
                "No",
                "Yes",
                [this](auto, bool btn2) {
                    if (!btn2) return;

                    GEODE_UNWRAP_OR_ELSE(trashDir, err, MoreIcons::createTrash())
                        return notify(NotificationIcon::Error, "Failed to create trash directory: {}", err);

                    moveIcon(trashDir, true);
                }
            );
        });
        operationButton->setID("trash-button");
    }

    if (operationButton) {
        operationButton->setPosition({ 275.0f, 25.0f });
        m_buttonMenu->addChild(operationButton);
    }

    auto okButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("OK"), [this](auto) {
        onClose(nullptr);
    });
    okButton->setPosition({ 150.0f, 25.0f });
    okButton->setID("ok-button");
    m_buttonMenu->addChild(okButton);

    return true;
}

void MoreInfoPopup::close() {
    onClose(nullptr);
}
