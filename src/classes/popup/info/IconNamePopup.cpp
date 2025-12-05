#include "IconNamePopup.hpp"
#include "MoreInfoPopup.hpp"
#include "../../../MoreIcons.hpp"
#include <fmt/std.h>
#include <Geode/binding/ButtonSprite.hpp>
#ifdef GEODE_IS_WINDOWS
#include <Geode/utils/string.hpp>
#endif
#include <Geode/ui/Notification.hpp>
#include <MoreIconsV2.hpp>

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

template <typename... T>
void notify(NotificationIcon icon, fmt::format_string<T...> message, T&&... args) {
    Notification::create(fmt::format(message, std::forward<T>(args)...), icon)->show();
}

bool IconNamePopup::setup(MoreInfoPopup* popup, IconInfo* info) {
    auto unlockName = MoreIcons::uppercase[MoreIcons::convertType(info->type)];

    setID("IconNamePopup");
    setTitle(fmt::format("Edit {} Name", unlockName));
    m_title->setID("icon-name-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_iconType = info->type;
    m_info = info;

    m_nameInput = TextInput::create(300.0f, "Name");
    m_nameInput->setPosition({ 175.0f, 70.0f });
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_nameInput->setMaxCharCount(100);
    m_nameInput->setString(info->shortName);
    m_nameInput->setID("name-input");
    m_mainLayer->addChild(m_nameInput);

    auto confirmButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Confirm", 0.8f), [this, info, popup, unlockName](auto) {
        auto name = m_nameInput->getString();
        if (name.empty()) return notify(NotificationIcon::Info, "Name cannot be empty.");

        auto& old = info->shortName;
        if (name == old) return notify(NotificationIcon::Info, "Name is already set to {}.", name);

        fmt::memory_buffer message;
        fmt::format_to(std::back_inserter(message), "Are you sure you want to rename <cy>{}</c> to <cy>{}</c>?", old, name);

        auto parent = MoreIcons::strPath(info->textures[0]).parent_path();
        fmt::memory_buffer files;

        #ifdef GEODE_IS_WINDOWS
        auto wideOld = string::utf8ToWide(old);
        auto wideName = string::utf8ToWide(name);
        #else
        auto& wideOld = old;
        auto& wideName = name;
        #endif
        if (info->type == IconType::Special) {
            if (doesExist(parent, wideOld + MI_PATH(".png"), wideName + MI_PATH(".png"))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}.png</c>", name);
            }
            if (doesExist(parent, wideOld + MI_PATH(".json"), wideName + MI_PATH(".json"))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}.json</c>", name);
            }
        }
        else if (info->type <= IconType::Jetpack) {
            if (doesExist(parent, wideOld + MI_PATH("-uhd.png"), wideName + MI_PATH("-uhd.png"))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-uhd.png</c>", name);
            }
            if (doesExist(parent, wideOld + MI_PATH("-hd.png"), wideName + MI_PATH("-hd.png"))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-hd.png</c>", name);
            }
            if (doesExist(parent, wideOld + MI_PATH(".png"), wideName + MI_PATH(".png"))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}.png</c>", name);
            }
            if (doesExist(parent, wideOld + MI_PATH("-uhd.plist"), wideName + MI_PATH("-uhd.plist"))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-uhd.plist</c>", name);
            }
            if (doesExist(parent, wideOld + MI_PATH("-hd.plist"), wideName + MI_PATH("-hd.plist"))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-hd.plist</c>", name);
            }
            if (doesExist(parent, wideOld + MI_PATH(".plist"), wideName + MI_PATH(".plist"))) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}.plist</c>", name);
            }
        }

        if (files.size() > 0) {
            fmt::format_to(std::back_inserter(message),
                "\n<cr>This will overwrite the following files:</c>{}\n<cr>These cannot be restored!</c>", fmt::to_string(files));
        }

        createQuickPopup(fmt::format("Rename {}", unlockName).c_str(), fmt::to_string(message), "No", "Yes", [
            this, info, old, name, wideOld = GEODE_WINDOWS(std::move)(wideOld), wideName = GEODE_WINDOWS(std::move)(wideName), popup
        ](auto, bool btn2) {
            if (!btn2) return;

            auto parent = MoreIcons::strPath(info->textures[0]).parent_path();

            if (info->type == IconType::Special) {
                if (auto res = renameFile(parent, wideOld + MI_PATH(".png"), wideName + MI_PATH(".png")); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to rename {}.png: {}", old, res.unwrapErr());
                }
                if (auto res = renameFile(parent, wideOld + MI_PATH(".json"), wideName + MI_PATH(".json")); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to rename {}.json: {}", old, res.unwrapErr());
                }
            }
            else if (info->type <= IconType::Jetpack) {
                if (auto res = renameFile(parent, wideOld + MI_PATH("-uhd.png"), wideName + MI_PATH("-uhd.png")); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to rename {}-uhd.png: {}", old, res.unwrapErr());
                }
                if (auto res = renameFile(parent, wideOld + MI_PATH("-hd.png"), wideName + MI_PATH("-hd.png")); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to rename {}-hd.png: {}", old, res.unwrapErr());
                }
                if (auto res = renameFile(parent, wideOld + MI_PATH(".png"), wideName + MI_PATH(".png")); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to rename {}.png: {}", old, res.unwrapErr());
                }
                if (auto res = renameFile(parent, wideOld + MI_PATH("-uhd.plist"), wideName + MI_PATH("-uhd.plist")); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to rename {}-uhd.plist: {}", old, res.unwrapErr());
                }
                if (auto res = renameFile(parent, wideOld + MI_PATH("-hd.plist"), wideName + MI_PATH("-hd.plist")); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to rename {}-hd.plist: {}", old, res.unwrapErr());
                }
                if (auto res = renameFile(parent, wideOld + MI_PATH(".plist"), wideName + MI_PATH(".plist")); res.isErr()) {
                    return notify(NotificationIcon::Error, "Failed to rename {}.plist: {}", old, res.unwrapErr());
                }
            }

            Popup::onClose(nullptr);
            popup->close();
            more_icons::renameIcon(info, name);
            notify(NotificationIcon::Success, "{} renamed to {}!", old, name);
            MoreIcons::updateGarage();
        });
    });
    confirmButton->setPosition({ 175.0f, 30.0f });
    confirmButton->setID("confirm-button");
    m_buttonMenu->addChild(confirmButton);

    handleTouchPriority(this);

    return true;
}

void IconNamePopup::onClose(CCObject* sender) {
    auto name = m_nameInput->getString();
    if (name.empty() || name == m_info->shortName) return Popup::onClose(sender);

    auto type = MoreIcons::convertType(m_iconType);
    createQuickPopup(
        fmt::format("Exit {} Name Editor", MoreIcons::uppercase[type]).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} name editor</c>?", MoreIcons::lowercase[type]),
        "No",
        "Yes",
        [this, sender](auto, bool btn2) {
            if (btn2) Popup::onClose(sender);
        }
    );
}
