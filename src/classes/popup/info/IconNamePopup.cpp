#include "IconNamePopup.hpp"
#include "MoreInfoPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <fmt/std.h>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

IconNamePopup* IconNamePopup::create(MoreInfoPopup* popup, IconInfo* info) {
    auto ret = new IconNamePopup();
    if (ret->initAnchored(350.0f, 130.0f, popup, info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool doesExist(const std::filesystem::path& parent, const std::filesystem::path& file1, const std::filesystem::path& file2) {
    return MoreIcons::doesExist(parent / file1) && MoreIcons::doesExist(parent / file2);
}

Result<> renameFile(const std::filesystem::path& parent, const std::filesystem::path& from, const std::filesystem::path& to) {
    return MoreIcons::renameFile(parent / from, parent / to);
}

bool IconNamePopup::setup(MoreInfoPopup* popup, IconInfo* info) {
    auto unlockName = MoreIconsAPI::uppercase[MoreIconsAPI::convertType(info->type)];

    setID("IconNamePopup");
    setTitle(fmt::format("Edit {} Name", unlockName));
    m_title->setID("icon-name-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_iconType = info->type;

    m_nameInput = TextInput::create(300.0f, "Name");
    m_nameInput->setPosition({ 175.0f, 70.0f });
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_nameInput->setMaxCharCount(100);
    m_nameInput->setID("name-input");
    m_mainLayer->addChild(m_nameInput);

    auto confirmButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Confirm", 0.8f), [this, info, popup, unlockName](auto) {
        auto name = m_nameInput->getString();
        if (name.empty()) {
            return Notification::create("Name cannot be empty.", NotificationIcon::Info)->show();
        }

        auto& old = info->shortName;
        if (name == old) {
            return Notification::create(fmt::format("Name is already set to {}.", name), NotificationIcon::Info)->show();
        }

        fmt::memory_buffer message;
        fmt::format_to(std::back_inserter(message), "Are you sure you want to rename <cy>{}</c> to <cy>{}</c>?", old, name);

        auto parent = std::filesystem::path(info->textures[0]).parent_path();
        fmt::memory_buffer files;

        if (info->type == IconType::Special) {
            if (doesExist(parent, fmt::format("{}.png", old), fmt::format("{}.png", name))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}.png</c>", name);
            }
            if (doesExist(parent, fmt::format("{}.json", old), fmt::format("{}.json", name))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}.json</c>", name);
            }
        }
        else if (info->type <= IconType::Jetpack) {
            if (doesExist(parent, fmt::format("{}-uhd.png", old), fmt::format("{}-uhd.png", name))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-uhd.png</c>", name);
            }
            if (doesExist(parent, fmt::format("{}-hd.png", old), fmt::format("{}-hd.png", name))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-hd.png</c>", name);
            }
            if (doesExist(parent, fmt::format("{}.png", old), fmt::format("{}.png", name))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}.png</c>", name);
            }
            if (doesExist(parent, fmt::format("{}-uhd.plist", old), fmt::format("{}-uhd.plist", name))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-uhd.plist</c>", name);
            }
            if (doesExist(parent, fmt::format("{}-hd.plist", old), fmt::format("{}-hd.plist", name))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-hd.plist</c>", name);
            }
            if (doesExist(parent, fmt::format("{}.plist", old), fmt::format("{}.plist", name))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}.plist</c>", name);
            }
        }

        if (files.size() > 0) {
            fmt::format_to(std::back_inserter(message),
                "\n<cr>This will overwrite the following files:</c>{}\n<cr>These cannot be restored!</c>", fmt::to_string(files));
        }

        createQuickPopup(fmt::format("Rename {}", unlockName).c_str(), fmt::to_string(message), "No", "Yes", [
            this, info, old, name, popup
        ](auto, bool btn2) {
            if (!btn2) return;

            auto parent = std::filesystem::path(info->textures[0]).parent_path();

            if (info->type == IconType::Special) {
                if (auto res = renameFile(parent, fmt::format("{}.png", old), fmt::format("{}.png", name)); res.isErr()) {
                    return Notification::create(
                        fmt::format("Failed to rename {}.png: {}", old, res.unwrapErr()), NotificationIcon::Error)->show();
                }
                if (auto res = renameFile(parent, fmt::format("{}.json", old), fmt::format("{}.json", name)); res.isErr()) {
                    return Notification::create(
                        fmt::format("Failed to rename {}.json: {}", old, res.unwrapErr()), NotificationIcon::Error)->show();
                }
            }
            else if (info->type <= IconType::Jetpack) {
                if (auto res = renameFile(parent, fmt::format("{}-uhd.png", old), fmt::format("{}-uhd.png", name)); res.isErr()) {
                    return Notification::create(
                        fmt::format("Failed to rename {}-uhd.png: {}", old, res.unwrapErr()), NotificationIcon::Error)->show();
                }
                if (auto res = renameFile(parent, fmt::format("{}-hd.png", old), fmt::format("{}-hd.png", name)); res.isErr()) {
                    return Notification::create(
                        fmt::format("Failed to rename {}-hd.png: {}", old, res.unwrapErr()), NotificationIcon::Error)->show();
                }
                if (auto res = renameFile(parent, fmt::format("{}.png", old), fmt::format("{}.png", name)); res.isErr()) {
                    return Notification::create(
                        fmt::format("Failed to rename {}.png: {}", old, res.unwrapErr()), NotificationIcon::Error)->show();
                }
                if (auto res = renameFile(parent, fmt::format("{}-uhd.plist", old), fmt::format("{}-uhd.plist", name)); res.isErr()) {
                    return Notification::create(
                        fmt::format("Failed to rename {}-uhd.plist: {}", old, res.unwrapErr()), NotificationIcon::Error)->show();
                }
                if (auto res = renameFile(parent, fmt::format("{}-hd.plist", old), fmt::format("{}-hd.plist", name)); res.isErr()) {
                    return Notification::create(
                        fmt::format("Failed to rename {}-hd.plist: {}", old, res.unwrapErr()), NotificationIcon::Error)->show();
                }
                if (auto res = renameFile(parent, fmt::format("{}.plist", old), fmt::format("{}.plist", name)); res.isErr()) {
                    return Notification::create(
                        fmt::format("Failed to rename {}.plist: {}", old, res.unwrapErr()), NotificationIcon::Error)->show();
                }
            }

            Popup::onClose(nullptr);
            popup->close();
            MoreIconsAPI::renameIcon(info, name);
            Notification::create(fmt::format("{} renamed to {}!", old, name), NotificationIcon::Success)->show();
            MoreIcons::updateGarage();
        });
    });
    confirmButton->setPosition({ 175.0f, 30.0f });
    confirmButton->setID("confirm-button");
    m_buttonMenu->addChild(confirmButton);

    handleTouchPriority(this);

    return true;
}

void IconNamePopup::onClose(cocos2d::CCObject* sender) {
    if (m_nameInput->getString().empty()) return Popup::onClose(sender);

    auto type = MoreIconsAPI::convertType(m_iconType);
    createQuickPopup(
        fmt::format("Exit {} Name Editor", MoreIconsAPI::uppercase[type]).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} name editor</c>?\n<cr>All unsaved changes will be lost!</c>",
            MoreIconsAPI::lowercase[type]),
        "No",
        "Yes",
        [this, sender](auto, bool btn2) {
            if (btn2) Popup::onClose(sender);
        }
    );
}
