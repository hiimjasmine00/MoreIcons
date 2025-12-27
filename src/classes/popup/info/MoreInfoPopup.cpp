#include "MoreInfoPopup.hpp"
#include "IconNamePopup.hpp"
#include "SpecialSettingsPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Icons.hpp"
#include "../../../utils/Load.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/TextArea.hpp>
#include <Geode/loader/Dirs.hpp>
#include <MoreIcons.hpp>

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

Result<> copyVanillaFile(const std::filesystem::path& src, const std::filesystem::path& dest) {
    GEODE_UNWRAP_INTO(auto vec, Load::readBinary(src).mapErr([&src](std::string err) {
        return fmt::format("Failed to read {}: {}", Filesystem::filenameFormat(src), err);
    }));
    return file::writeBinary(dest, vec).mapErr([&dest](std::string err) {
        return fmt::format("Failed to write {}: {}", Filesystem::filenameFormat(dest), err);
    });
}

Result<> copyFile(const std::filesystem::path& src, const std::filesystem::path& dest) {
    if (!Filesystem::doesExist(src)) return Ok();
    GEODE_UNWRAP_INTO(auto vec, file::readBinary(src).mapErr([&src](std::string err) {
        return fmt::format("Failed to read {}: {}", Filesystem::filenameFormat(src), err);
    }));
    return file::writeBinary(dest, vec).mapErr([&dest](std::string err) {
        return fmt::format("Failed to write {}: {}", Filesystem::filenameFormat(dest), err);
    });
}

bool moveSheet(Filesystem::PathView name, Filesystem::PathView from, Filesystem::PathView to, Filesystem::PathView parent, bool trash) {
    auto resources = dirs::getResourcesDir();
    if (!parent.empty()) resources = std::move(resources) / parent;
    resources = std::move(resources) / name;

    std::vector<std::pair<std::filesystem::path, std::filesystem::path>> files;

    std::filesystem::path uhdPng = fmt::format(L("{}-uhd.png"), from);
    if (Filesystem::doesExist(uhdPng)) {
        files.emplace_back(std::move(uhdPng), fmt::format(L("{}-uhd.png"), to));
        std::filesystem::path fromPlist = fmt::format(L("{}-uhd.plist"), from);
        std::filesystem::path toPlist = fmt::format(L("{}-uhd.plist"), to);
        if (Filesystem::doesExist(fromPlist)) files.emplace_back(std::move(fromPlist), std::move(toPlist));
        else if (!trash) {
            #ifdef GEODE_IS_DESKTOP
            if (auto res = copyVanillaFile(fmt::format(L("{}-uhd.plist"), resources), toPlist); res.isErr()) {
                Notify::error(res.unwrapErr());
                return false;
            }
            #else
            auto uhdResources = Icons::getUhdResourcesDir();
            if (!parent.empty()) uhdResources = std::move(uhdResources) / parent;
            uhdResources = std::move(uhdResources) / name;
            if (auto res = copyVanillaFile(fmt::format(L("{}-uhd.plist"), uhdResources), toPlist); res.isErr()) {
                files.pop_back();
            }
            #endif
        }
    }

    std::filesystem::path hdPng = fmt::format(L("{}-hd.png"), from);
    if (Filesystem::doesExist(hdPng)) {
        files.emplace_back(std::move(hdPng), fmt::format(L("{}-hd.png"), to));
        std::filesystem::path fromPlist = fmt::format(L("{}-hd.plist"), from);
        std::filesystem::path toPlist = fmt::format(L("{}-hd.plist"), to);
        if (Filesystem::doesExist(fromPlist)) files.emplace_back(std::move(fromPlist), std::move(toPlist));
        else if (!trash) {
            if (auto res = copyVanillaFile(fmt::format(L("{}-hd.plist"), resources), toPlist); res.isErr()) {
                Notify::error(res.unwrapErr());
                return false;
            }
        }
    }

    std::filesystem::path png = fmt::format(L("{}.png"), from);
    if (Filesystem::doesExist(png)) {
        files.emplace_back(std::move(png), fmt::format(L("{}.png"), to));
        std::filesystem::path fromPlist = fmt::format(L("{}.plist"), from);
        std::filesystem::path toPlist = fmt::format(L("{}.plist"), to);
        if (Filesystem::doesExist(fromPlist)) files.emplace_back(std::move(fromPlist), std::move(toPlist));
        else if (!trash) {
            if (auto res = copyVanillaFile(fmt::format(L("{}.plist"), resources), toPlist); res.isErr()) {
                Notify::error(res.unwrapErr());
                return false;
            }
        }
    }

    if (files.empty()) {
        if (trash) Notify::info("No files found to trash.");
        else Notify::info("No files found to move.");
        return false;
    }

    for (auto it = files.begin(); it != files.end(); ++it) {
        if (auto res = Filesystem::renameFile(it->first, it->second); res.isErr()) {
            for (auto it2 = files.begin(); it2 != it; ++it2) {
                (void)Filesystem::renameFile(it2->second, it2->first);
            }
            Notify::error(res.unwrapErr());
            return false;
        }
    }

    return true;
}

void MoreInfoPopup::moveIcon(const std::filesystem::path& directory, bool trash) {
    auto type = m_info->getType();

    auto& texture = m_info->getTexture();
    auto parentDir = Filesystem::parentPath(texture);
    auto& shortName = m_info->getShortName();
    auto stem = Filesystem::strWide(shortName);
    if (type >= IconType::DeathEffect) {
        if (trash) {
            if (auto res = Filesystem::renameFile(parentDir, directory / Filesystem::filenameView(parentDir)); res.isErr()) {
                return Notify::error(res.unwrapErr());
            }
        }
        else {
            if (auto res = file::createDirectoryAll(directory); res.isErr()) {
                return Notify::error(res.unwrapErr());
            }

            if (type == IconType::ShipFire) {
                if (auto res = Filesystem::renameFile(texture, directory / L("fire_001.png")); res.isErr()) {
                    return Notify::error(res.unwrapErr());
                }
            }
            else if (type == IconType::Special) {
                if (auto res = Filesystem::renameFile(texture, directory / L("trail.png")); res.isErr()) {
                    return Notify::error(res.unwrapErr());
                }
            }
            else if (type == IconType::DeathEffect) {
                if (!moveSheet(stem, (std::move(parentDir) / stem).native(), (directory / L("effect")).native(), {}, false)) return;
            }

            auto iconStem = Mod::get()->getResourcesDir() / fmt::format(L("{}{:02}"), Constants::getFolderName(type), m_info->getSpecialID());

            if (auto res = copyFile(fmt::format(L("{}-uhd.png"), iconStem), directory / L("icon-uhd.png")); res.isErr()) {
                return Notify::error(res.unwrapErr());
            }

            if (auto res = copyFile(fmt::format(L("{}-hd.png"), iconStem), directory / L("icon-hd.png")); res.isErr()) {
                return Notify::error(res.unwrapErr());
            }

            if (auto res = copyFile(fmt::format(L("{}.png"), iconStem), directory / L("icon.png")); res.isErr()) {
                return Notify::error(res.unwrapErr());
            }
        }
    }
    else if (type <= IconType::Jetpack) {
        if (!moveSheet(stem, (std::move(parentDir) / stem).native(), (directory / stem).native(), L("icons"), trash)) return;
    }

    auto name = shortName;
    onClose(nullptr);
    if (trash) {
        more_icons::removeIcon(m_info);
        Notify::success("{} trashed!", name);
    }
    else {
        more_icons::moveIcon(m_info, directory);
        Notify::success("{} moved!", name);
    }
    MoreIcons::updateGarage();
}

bool MoreInfoPopup::setup(IconInfo* info) {
    setID("MoreInfoPopup");
    setTitle(info->getShortName(), "goldFont.fnt", 0.8f, 16.0f);
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

    auto hasPack = info->inTexturePack();
    auto type = info->getType();

    auto customLabel = CCLabelBMFont::create("Custom", "goldFont.fnt");
    customLabel->setPosition({ 150.0f, 123.0f });
    customLabel->setScale(0.55f);
    customLabel->setID("custom-label");
    m_mainLayer->addChild(customLabel);

    auto descriptionArea = TextArea::create(fmt::format("This <cg>{}</c> is added by the <cl>More Icons</c> mod.",
        Constants::getSingularUppercase(type)), "bigFont.fnt", 1.0f, 600.0f, { 0.5f, 1.0f }, 42.0f, false);
    descriptionArea->setPosition({ 150.0f, 91.0f });
    descriptionArea->setScale(0.4f);
    descriptionArea->setID("description-area");
    m_mainLayer->addChild(descriptionArea);

    if (hasPack) {
        auto packLabel = CCLabelBMFont::create(info->getPackName().c_str(), "goldFont.fnt");
        packLabel->setPosition({ 150.0f, 198.0f });
        packLabel->setScale(0.4f);
        packLabel->setID("pack-label");
        m_mainLayer->addChild(packLabel);
    }

    if (type <= IconType::Jetpack) {
        auto itemIcon = GJItemIcon::createBrowserItem(Get::GameManager()->iconTypeToUnlockType(type), 1);
        itemIcon->setScale(hasPack ? 1.1f : 1.25f);

        auto player = static_cast<SimplePlayer*>(itemIcon->m_player);
        more_icons::updateSimplePlayer(player, info->getName(), type);

        auto dual = MoreIcons::dualSelected();
        auto color1 = MoreIcons::vanillaColor1(dual);
        auto color2 = MoreIcons::vanillaColor2(dual);
        auto colorGlow = MoreIcons::vanillaColorGlow(dual);
        auto glow = MoreIcons::vanillaGlow(dual);
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
    else if (type >= IconType::DeathEffect) {
        auto sprite = MoreIcons::customIcon(info);
        sprite->setPosition({ 150.0f, hasPack ? 165.0f : 171.0f });
        sprite->setScale(hasPack ? 1.1f : 1.25f);
        sprite->setID("custom-icon");
        m_mainLayer->addChild(sprite);

        if (info->getSpecialID() == 0) {
            auto settingsButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_optionsBtn_001.png", 0.7f, [info](auto) {
                SpecialSettingsPopup::create(info)->show();
            });
            settingsButton->setPosition({ 25.0f, 25.0f });
            settingsButton->setID("settings-button");
            m_buttonMenu->addChild(settingsButton);
        }
    }

    CCMenuItemSpriteExtra* operationButton = nullptr;

    if (info->isVanilla() && !info->isZipped()) {
        operationButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.7f, [this, type](auto) {
            auto& shortName = m_info->getShortName();
            auto lower = Constants::getSingularLowercase(type);

            fmt::memory_buffer message;
            fmt::format_to(std::back_inserter(message),
                "Are you sure you want to <cy>convert</c> <cj>{}</c> into a <cl>More Icons</c> <cg>{}</c>?", shortName, lower);

            auto dir = Filesystem::parentPath(m_info->getTexture());
            if (type <= IconType::Jetpack) dir = Filesystem::parentPath(std::move(dir));
            dir = std::move(dir) / CONFIG_PATH / Constants::getFolderName(type);
            if (type >= IconType::DeathEffect) {
                dir = std::move(dir) / Filesystem::strWide(shortName);
                if (Filesystem::doesExist(dir)) {
                    fmt::format_to(std::back_inserter(message), "\n<cr>This will overwrite the existing custom {}!</c>", lower);
                }
            }

            createQuickPopup(fmt::format("Convert {}", Constants::getSingularUppercase(type)).c_str(), fmt::to_string(message), "No", "Yes", [
                this, dir = std::move(dir)
            ](auto, bool btn2) {
                if (!btn2) return;

                if (auto res = file::createDirectoryAll(dir)) moveIcon(dir, false);
                else Notify::error(res.unwrapErr());
            });
        });
        operationButton->setID("move-button");
    }
    else if (!info->isZipped()) {
        m_title->setPosition({ 140.0f, m_size.height - 16.0f });

        auto editButton = CCMenuItemExt::createSpriteExtraWithFilename("MI_pencil_001.png"_spr, 0.7f, [this, info](auto) {
            IconNamePopup::create(this, info)->show();
        });
        editButton->setPosition(m_title->getPosition() + CCPoint { m_title->getScaledContentWidth() / 2.0f + 10.0f, 0.0f });
        editButton->setID("edit-button");
        m_buttonMenu->addChild(editButton);

        operationButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_trashBtn_001.png", 0.8f, [this, type](auto) {
            createQuickPopup(
                fmt::format("Trash {}", Constants::getSingularUppercase(type)).c_str(),
                fmt::format("Are you sure you want to <cr>trash</c> this <cg>{}</c>?", Constants::getSingularLowercase(type)),
                "No",
                "Yes",
                [this](auto, bool btn2) {
                    if (!btn2) return;

                    if (auto res = MoreIcons::createTrash()) moveIcon(res.unwrap(), true);
                    else Notify::error(res.unwrapErr());
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

    handleTouchPriority(this);

    return true;
}

void MoreInfoPopup::close() {
    onClose(nullptr);
}
