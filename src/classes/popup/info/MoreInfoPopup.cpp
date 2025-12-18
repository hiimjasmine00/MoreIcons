#define FMT_CPP_LIB_FILESYSTEM 0
#include "MoreInfoPopup.hpp"
#include "IconNamePopup.hpp"
#include "SpecialSettingsPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Load.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/TextArea.hpp>
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

Result<> copyVanillaFile(const std::filesystem::path& src, const std::filesystem::path& dest, bool uhd) {
    GEODE_UNWRAP_INTO(auto vec, Load::readBinary(src).mapErr([&src](std::string err) {
        return fmt::format("Failed to read {}: {}", src.filename(), err);
    }));
    return file::writeBinary(dest, vec).mapErr([&dest](std::string err) {
        return fmt::format("Failed to write {}: {}", dest.filename(), err);
    });
}

void MoreInfoPopup::moveIcon(const std::filesystem::path& directory, bool trash) {
    auto type = m_info->getType();

    if (type >= IconType::DeathEffect) {
        if (trash) {
            auto parentDir = m_info->getTexture().parent_path();
            auto filename = parentDir.filename();
            if (auto res = MoreIcons::renameFile(parentDir, directory / filename, false, true); res.isErr()) {
                return MoreIcons::notifyFailure(res.unwrapErr());
            }
        }
        else {
            auto shortName = MoreIcons::strWide(m_info->getShortName());
            auto iconDir = directory / shortName;
            if (auto res = file::createDirectoryAll(iconDir); res.isErr()) {
                return MoreIcons::notifyFailure(res.unwrapErr());
            }

            auto& texture = m_info->getTexture();
            auto mod = Mod::get();
            std::filesystem::path::string_type iconStem;
            if (type == IconType::ShipFire) {
                if (auto res = MoreIcons::renameFile(texture, iconDir / L("fire_001.png"), false, true); res.isErr()) {
                    return MoreIcons::notifyFailure(res.unwrapErr());
                }

                iconStem = MoreIcons::getPathString(mod->getResourcesDir() / fmt::format(L("shipfireIcon_{:02}_001"), m_info->getSpecialID()));
            }
            else if (type == IconType::Special) {
                if (auto res = MoreIcons::renameFile(texture, iconDir / L("trail.png"), false, true); res.isErr()) {
                    return MoreIcons::notifyFailure(res.unwrapErr());
                }

                iconStem = MoreIcons::getPathString(mod->getResourcesDir() / fmt::format(L("player_special_{:02}_001"), m_info->getSpecialID()));
            }
            else if (type == IconType::DeathEffect) {
                auto parentDir = texture.parent_path();
                auto stem = MoreIcons::getPathString(parentDir / shortName);
                auto resources = MoreIcons::getResourcesDir(false);
                std::vector<std::filesystem::path> files;

                std::filesystem::path uhdPng = fmt::format(L("{}-uhd.png"), stem);
                if (MoreIcons::doesExist(uhdPng)) {
                    files.push_back(std::move(uhdPng));
                    std::filesystem::path filename = fmt::format(L("{}-uhd.plist"), shortName);
                    auto plist = parentDir / filename;
                    if (MoreIcons::doesExist(plist)) files.push_back(std::move(plist));
                    else {
                        #ifdef GEODE_IS_DESKTOP
                        if (auto res = copyVanillaFile(resources / filename, directory / filename, true); res.isErr()) {
                            return MoreIcons::notifyFailure(res.unwrapErr());
                        }
                        #else
                        if (auto res = copyVanillaFile(MoreIcons::getResourcesDir(true) / filename, directory / filename, true); res.isErr()) {
                            files.pop_back();
                        }
                        #endif
                    }
                }

                std::filesystem::path hdPng = fmt::format(L("{}-hd.png"), stem);
                if (MoreIcons::doesExist(hdPng)) {
                    files.push_back(std::move(hdPng));
                    std::filesystem::path filename = fmt::format(L("{}-hd.plist"), shortName);
                    auto plist = parentDir / filename;
                    if (MoreIcons::doesExist(plist)) files.push_back(std::move(plist));
                    else if (auto res = copyVanillaFile(resources / filename, directory / filename, false); res.isErr()) {
                        return MoreIcons::notifyFailure(res.unwrapErr());
                    }
                }

                std::filesystem::path png = fmt::format(L("{}.png"), stem);
                if (MoreIcons::doesExist(png)) {
                    files.push_back(std::move(png));
                    std::filesystem::path filename = fmt::format(L("{}.plist"), shortName);
                    auto plist = parentDir / filename;
                    if (MoreIcons::doesExist(plist)) files.push_back(std::move(plist));
                    else if (auto res = copyVanillaFile(resources / filename, directory / filename, false); res.isErr()) {
                        return MoreIcons::notifyFailure(res.unwrapErr());
                    }
                }

                if (files.empty()) return MoreIcons::notifyInfo("No files found to move.");

                std::vector<std::filesystem::path> filenames;
                for (auto& file : files) {
                    auto filename = file.filename();
                    if (auto res = MoreIcons::renameFile(file, directory / filename, false, true)) {
                        filenames.push_back(std::move(res).unwrap());
                    }
                    else if (res.isErr()) {
                        for (size_t i = 0; i < filenames.size(); i++) {
                            (void)MoreIcons::renameFile(directory / filenames[i].filename(), files[i], false);
                        }
                        return MoreIcons::notifyFailure(res.unwrapErr());
                    }
                }

                iconStem = MoreIcons::getPathString(mod->getResourcesDir() / fmt::format(L("explosionIcon_{:02}_001"), m_info->getSpecialID()));
            }

            constexpr std::array suffixes = { L(".png"), L("-hd.png"), L("-uhd.png") };

            for (auto suffix : suffixes) {
                std::filesystem::path iconPath = fmt::format(L("{}{}"), iconStem, suffix);
                if (MoreIcons::doesExist(iconPath)) {
                    auto readRes = file::readBinary(iconPath);
                    if (readRes.isErr()) {
                        return MoreIcons::notifyFailure("Failed to read {}: {}", iconPath.filename(), readRes.unwrapErr());
                    }

                    if (auto writeRes = file::writeBinary(iconDir / fmt::format(L("icon{}"), suffix), readRes.unwrap()); writeRes.isErr()) {
                        return MoreIcons::notifyFailure("Failed to write {}: {}", iconPath.filename(), writeRes.unwrapErr());
                    }
                }
            }
        }
    }
    else if (type <= IconType::Jetpack) {
        auto shortName = MoreIcons::strWide(m_info->getShortName());
        auto parentDir = m_info->getTexture().parent_path();
        auto stem = MoreIcons::getPathString(parentDir / shortName);
        auto resources = MoreIcons::getResourcesDir(false);
        std::vector<std::filesystem::path> files;

        std::filesystem::path uhdPng = fmt::format(L("{}-uhd.png"), stem);
        if (MoreIcons::doesExist(uhdPng)) {
            files.push_back(std::move(uhdPng));
            std::filesystem::path filename = fmt::format(L("{}-uhd.plist"), shortName);
            auto plist = parentDir / filename;
            if (MoreIcons::doesExist(plist)) files.push_back(std::move(plist));
            else if (!trash) {
                #ifdef GEODE_IS_DESKTOP
                if (auto res = copyVanillaFile(resources / L("icons") / filename, directory / filename, true); res.isErr()) {
                    return MoreIcons::notifyFailure(res.unwrapErr());
                }
                #else
                if (auto res = copyVanillaFile(MoreIcons::getResourcesDir(true) / L("icons") / filename, directory / filename, true); res.isErr()) {
                    files.pop_back();
                }
                #endif
            }
        }

        std::filesystem::path hdPng = fmt::format(L("{}-hd.png"), stem);
        if (MoreIcons::doesExist(hdPng)) {
            files.push_back(std::move(hdPng));
            std::filesystem::path filename = fmt::format(L("{}-hd.plist"), shortName);
            auto plist = parentDir / filename;
            if (MoreIcons::doesExist(plist)) files.push_back(std::move(plist));
            else if (!trash) {
                if (auto res = copyVanillaFile(resources / L("icons") / filename, directory / filename, false); res.isErr()) {
                    return MoreIcons::notifyFailure(res.unwrapErr());
                }
            }
        }

        std::filesystem::path png = fmt::format(L("{}.png"), stem);
        if (MoreIcons::doesExist(png)) {
            files.push_back(std::move(png));
            std::filesystem::path filename = fmt::format(L("{}.plist"), shortName);
            auto plist = parentDir / filename;
            if (MoreIcons::doesExist(plist)) files.push_back(std::move(plist));
            else if (!trash) {
                if (auto res = copyVanillaFile(resources / L("icons") / filename, directory / filename, false); res.isErr()) {
                    return MoreIcons::notifyFailure(res.unwrapErr());
                }
            }
        }

        if (files.empty()) {
            return MoreIcons::notifyInfo("No files found to {}.", trash ? "trash" : "move");
        }

        std::vector<std::filesystem::path> filenames;
        for (auto& file : files) {
            auto filename = file.filename();
            if (auto res = MoreIcons::renameFile(file, directory / filename, false, true)) {
                filenames.push_back(std::move(res).unwrap());
            }
            else if (res.isErr()) {
                for (size_t i = 0; i < filenames.size(); i++) {
                    (void)MoreIcons::renameFile(directory / filenames[i].filename(), files[i], false);
                }
                return MoreIcons::notifyFailure(res.unwrapErr());
            }
        }
    }

    auto name = m_info->getShortName();
    onClose(nullptr);
    if (trash) more_icons::removeIcon(m_info);
    else more_icons::moveIcon(m_info, directory);
    MoreIcons::notifySuccess("{} {}ed!", name, trash ? "trash" : "convert");
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
    auto miType = MoreIcons::convertType(type);

    auto customLabel = CCLabelBMFont::create("Custom", "goldFont.fnt");
    customLabel->setPosition({ 150.0f, 123.0f });
    customLabel->setScale(0.55f);
    customLabel->setID("custom-label");
    m_mainLayer->addChild(customLabel);

    auto descriptionArea = TextArea::create(fmt::format("This <cg>{}</c> is added by the <cl>More Icons</c> mod.",
        MoreIcons::uppercase[miType]), "bigFont.fnt", 1.0f, 600.0f, { 0.5f, 1.0f }, 42.0f, false);
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
    else if (type >= IconType::Special) {
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
        operationButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.7f, [this, miType](auto) {
            auto lower = MoreIcons::lowercase[miType];
            createQuickPopup(
                fmt::format("Convert {}", MoreIcons::uppercase[miType]).c_str(),
                fmt::format("Are you sure you want to <cy>convert</c> this <cg>{}</c> into a <cl>More Icons</c> <cg>{}</c>?", lower, lower),
                "No",
                "Yes",
                [this, miType](auto, bool btn2) {
                    if (!btn2) return;

                    auto type = m_info->getType();
                    auto parent = m_info->getTexture().parent_path();
                    if (type <= IconType::Jetpack) parent = parent.parent_path();
                    auto dir = std::move(parent) / WIDE_CONFIG / MoreIcons::folders[miType];
                    if (auto res = file::createDirectoryAll(dir)) moveIcon(dir, false);
                    else MoreIcons::notifyFailure(res.unwrapErr());
                }
            );
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

        operationButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_trashBtn_001.png", 0.8f, [this, miType](auto) {
            createQuickPopup(
                fmt::format("Trash {}", MoreIcons::uppercase[miType]).c_str(),
                fmt::format("Are you sure you want to <cr>trash</c> this <cg>{}</c>?", MoreIcons::lowercase[miType]),
                "No",
                "Yes",
                [this](auto, bool btn2) {
                    if (!btn2) return;

                    if (auto res = MoreIcons::createTrash()) moveIcon(res.unwrap(), true);
                    else MoreIcons::notifyFailure(res.unwrapErr());
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
