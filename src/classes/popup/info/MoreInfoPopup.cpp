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

void notify(NotificationIcon icon, const std::string& message) {
    Notification::create(message, icon)->show();
}

Result<> copyVanillaFile(const std::filesystem::path& src, const std::filesystem::path& dest, bool uhd) {
    #ifdef GEODE_IS_MOBILE
    auto fullSrc = (uhd ? dirs::getModConfigDir() / "weebify.high-graphics-android" / GEODE_GD_VERSION_STRING : dirs::getResourcesDir()) / src;
    #else
    auto fullSrc = dirs::getResourcesDir() / src;
    #endif
    std::vector<uint8_t> vec;
    #ifdef GEODE_IS_ANDROID
    if (uhd) {
        GEODE_UNWRAP_INTO(vec, file::readBinary(fullSrc).mapErr([](const std::string& err) {
            return fmt::format("Failed to read file: {}", err);
        }));
    }
    else {
        auto size = 0ul;
        auto data = MoreIconsAPI::getFileUtils()->getFileData(fullSrc.c_str(), "rb", &size);
        if (!data) return Err("Failed to read file");

        vec.assign(data, data + size);
        delete[] data;
    }
    #else
    GEODE_UNWRAP_INTO(vec, file::readBinary(fullSrc).mapErr([](const std::string& err) {
        return fmt::format("Failed to read file: {}", err);
    }));
    #endif
    return file::writeBinary(dest, vec).mapErr([](const std::string& err) {
        return fmt::format("Failed to write file: {}", err);
    });
}

void MoreInfoPopup::moveIcon(const std::filesystem::path& directory, bool trash) {
    std::filesystem::path texturePath = m_info->textures[0];
    auto parentDir = texturePath.parent_path();

    if (m_info->type == IconType::Special) {
        auto filename = texturePath.filename();
        if (auto res = MoreIcons::renameFile(texturePath, directory / filename, false, true); res.isErr()) {
            return notify(NotificationIcon::Error, "Failed to {} {}: {}", trash ? "trash" : "move", filename, res.unwrapErr());
        }
        auto jsonName = texturePath.filename().replace_extension(".json");
        auto jsonPath = parentDir / jsonName;
        if (trash) {
            if (MoreIcons::doesExist(jsonPath)) {
                if (auto res = MoreIcons::renameFile(jsonPath, directory / jsonName, false, true); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to trash {}: {}", jsonName, res.unwrapErr());
                }
            }
        }
        else {
            if (auto res = file::writeToJson(jsonPath, m_info->trailInfo); res.isErr()) {
                return notify(NotificationIcon::Error, "Failed to write trail info to {}: {}", jsonName, res.unwrapErr());
            }
        }
    }
    else if (m_info->type <= IconType::Jetpack) {
        auto& shortName = m_info->shortName;
        std::vector<std::filesystem::path> files;

        auto uhdPng = parentDir / fmt::format("{}-uhd.png", shortName);
        if (MoreIcons::doesExist(uhdPng)) {
            files.push_back(uhdPng);
            auto filename = fmt::format("{}-uhd.plist", shortName);
            auto plist = parentDir / filename;
            if (MoreIcons::doesExist(plist)) files.push_back(plist);
            else if (!trash) {
                if (auto res = copyVanillaFile(fmt::format("icons/{}", filename), directory / filename, true); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to copy {}: {}", filename, res.unwrapErr());
                }
            }
        }

        auto hdPng = parentDir / fmt::format("{}-hd.png", shortName);
        if (MoreIcons::doesExist(hdPng)) {
            files.push_back(hdPng);
            auto filename = fmt::format("{}-hd.plist", shortName);
            auto plist = parentDir / filename;
            if (MoreIcons::doesExist(plist)) files.push_back(plist);
            else if (!trash) {
                if (auto res = copyVanillaFile(fmt::format("icons/{}", filename), directory / filename, false); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to copy {}: {}", filename, res.unwrapErr());
                }
            }
        }

        auto png = parentDir / fmt::format("{}.png", shortName);
        if (MoreIcons::doesExist(png)) {
            files.push_back(png);
            auto filename = fmt::format("{}.plist", shortName);
            auto plist = parentDir / filename;
            if (MoreIcons::doesExist(plist)) files.push_back(plist);
            else if (!trash) {
                if (auto res = copyVanillaFile(fmt::format("icons/{}", filename), directory / filename, false); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to copy {}: {}", filename, res.unwrapErr());
                }
            }
        }

        for (int i = 0; i < files.size(); i++) {
            auto& file = files[i];
            auto filename = file.filename();
            if (auto res = MoreIcons::renameFile(file, directory / filename, false, true); res.isErr()) {
                for (int j = 0; j < i; j++) {
                    auto& file2 = files[j];
                    (void)MoreIcons::renameFile(directory / file2.filename(), file2, false);
                }
                return notify(NotificationIcon::Error, "Failed to {} {}: {}", trash ? "trash" : "move", filename, res.unwrapErr());
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
    setTitle(info->shortName, "goldFont.fnt", 0.8f, 16.0f);
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
    auto miType = MoreIconsAPI::convertType(info->type);

    auto customLabel = CCLabelBMFont::create("Custom", "goldFont.fnt");
    customLabel->setPosition({ 150.0f, 123.0f });
    customLabel->setScale(0.55f);
    customLabel->setID("custom-label");
    m_mainLayer->addChild(customLabel);

    auto descriptionArea = TextArea::create(fmt::format("This <cg>{}</c> is added by the <cl>More Icons</c> mod.",
        MoreIconsAPI::uppercase[miType]), "bigFont.fnt", 1.0f, 600.0f, { 0.5f, 1.0f }, 42.0f, false);
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
        auto itemIcon = GJItemIcon::createBrowserItem(MoreIconsAPI::getGameManager()->iconTypeToUnlockType(info->type), 1);
        itemIcon->setScale(hasPack ? 1.1f : 1.25f);

        auto player = static_cast<SimplePlayer*>(itemIcon->m_player);
        MoreIconsAPI::updateSimplePlayer(player, info->name, info->type);

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto [color1, color2, colorGlow, glow] = MoreIcons::vanillaColors(sdi && sdi->getSavedValue("2pselected", false));
        player->enableCustomGlowColor(colorGlow);

        auto iconButton = CCMenuItemExt::createSpriteExtra(itemIcon, [this, color1, color2, glow](CCMenuItemSpriteExtra* sender) {
            m_toggled = !m_toggled;

            auto player = static_cast<SimplePlayer*>(static_cast<GJItemIcon*>(sender->getNormalImage())->m_player);
            player->m_firstLayer->setColor(m_toggled ? color1 : ccColor3B { 175, 175, 175 });
            player->m_secondLayer->setColor(m_toggled ? color2 : ccColor3B { 255, 255, 255 });
            player->m_hasGlowOutline = m_toggled && glow;
            player->updateColors();
        });
        iconButton->setPosition({ 150.0f, hasPack ? 165.0f : 171.0f });
        iconButton->setID("icon-button");
        m_buttonMenu->addChild(iconButton);
    }
    else if (info->type == IconType::Special) {
        auto square = MoreIconsAPI::customTrail(info->textures[0]);
        square->setPosition({ 150.0f, hasPack ? 165.0f : 171.0f });
        square->setScale(hasPack ? 1.1f : 1.25f);
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
        operationButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.7f, [this, miType](auto) {
            auto lower = MoreIconsAPI::lowercase[miType];
            createQuickPopup(
                fmt::format("Convert {}", MoreIconsAPI::uppercase[miType]).c_str(),
                fmt::format("Are you sure you want to <cy>convert</c> this <cg>{}</c> into a <cl>More Icons</c> <cg>{}</c>?", lower, lower),
                "No",
                "Yes",
                [this, miType](auto, bool btn2) {
                    if (!btn2) return;

                    auto type = m_info->type;
                    auto parent = std::filesystem::path(m_info->textures[0]).parent_path();
                    if (type <= IconType::Jetpack) parent = parent.parent_path();
                    auto dir = parent / "config" / GEODE_MOD_ID / MoreIcons::folders[miType];
                    if (auto res = file::createDirectoryAll(dir)) moveIcon(dir, false);
                    else notify(NotificationIcon::Error, res.unwrapErr());
                }
            );
        });
        operationButton->setID("move-button");
    }
    else if (!info->zipped) {
        m_title->setPosition({ 140.0f, m_size.height - 16.0f });

        auto editButton = CCMenuItemExt::createSpriteExtraWithFilename("MI_pencil_001.png"_spr, 0.7f, [this, info](auto) {
            IconNamePopup::create(this, info)->show();
        });
        editButton->setPosition(m_title->getPosition() + CCPoint { m_title->getScaledContentWidth() / 2.0f + 10.0f, 0.0f });
        editButton->setID("edit-button");
        m_buttonMenu->addChild(editButton);

        operationButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_trashBtn_001.png", 0.8f, [this, miType](auto) {
            createQuickPopup(
                fmt::format("Trash {}", MoreIconsAPI::uppercase[miType]).c_str(),
                fmt::format("Are you sure you want to <cr>trash</c> this <cg>{}</c>?", MoreIconsAPI::lowercase[miType]),
                "No",
                "Yes",
                [this](auto, bool btn2) {
                    if (!btn2) return;

                    if (auto res = MoreIcons::createTrash()) moveIcon(res.unwrap(), true);
                    else notify(NotificationIcon::Error, res.unwrapErr());
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
