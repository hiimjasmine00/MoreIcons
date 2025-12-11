#include "IconNamePopup.hpp"
#include "MoreInfoPopup.hpp"
#include "../../../MoreIcons.hpp"
#include <fmt/std.h>
#include <Geode/binding/ButtonSprite.hpp>
#ifdef GEODE_IS_WINDOWS
#include <Geode/utils/string.hpp>
#endif
#include <MoreIcons.hpp>

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

using path_view = std::basic_string_view<std::filesystem::path::value_type>;

void pushFile(fmt::memory_buffer& buffer, const std::filesystem::path& parent, path_view file1, path_view file2) {
    if (MoreIcons::doesExist(parent / file1) && MoreIcons::doesExist(parent / file2)) {
        fmt::format_to(std::back_inserter(buffer), "\n<cg>{}</c>", MoreIcons::strNarrow(file1));
    }
}

void renameFile(const std::filesystem::path& parent, path_view from, path_view to) {
    if (auto res = MoreIcons::renameFile(parent / from, parent / to); res.isErr()) {
        MoreIcons::notifyFailure("Failed to rename {}: {}", MoreIcons::strNarrow(from), res.unwrapErr());
    }
}

bool IconNamePopup::setup(MoreInfoPopup* popup, IconInfo* info) {
    auto unlockName = MoreIcons::uppercase[MoreIcons::convertType(info->getType())];

    setID("IconNamePopup");
    setTitle(fmt::format("Edit {} Name", unlockName));
    m_title->setID("icon-name-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_iconType = info->getType();
    m_info = info;

    m_nameInput = TextInput::create(300.0f, "Name");
    m_nameInput->setPosition({ 175.0f, 70.0f });
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_nameInput->setMaxCharCount(100);
    m_nameInput->setString(info->getShortName());
    m_nameInput->setID("name-input");
    m_mainLayer->addChild(m_nameInput);

    auto confirmButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Confirm", 0.8f), [this, info, popup, unlockName](auto) {
        auto name = m_nameInput->getString();
        if (name.empty()) return MoreIcons::notifyInfo("Name cannot be empty.");

        auto old = info->getShortName();
        if (name == old) return MoreIcons::notifyInfo("Name is already set to {}.", name);

        fmt::memory_buffer message;
        fmt::format_to(std::back_inserter(message), "Are you sure you want to rename <cy>{}</c> to <cy>{}</c>?", old, name);

        auto parent = info->getTexture().parent_path();
        fmt::memory_buffer files;

        auto wideOld = GEODE_WINDOWS(string::utf8ToWide)(old);
        auto wideName = GEODE_WINDOWS(string::utf8ToWide)(name);
        auto type = info->getType();
        if (type >= IconType::DeathEffect) {
            pushFile(files, parent, wideOld, wideName);
        }
        else if (type <= IconType::Jetpack) {
            pushFile(files, parent, wideOld + MI_PATH("-uhd.png"), wideName + MI_PATH("-uhd.png"));
            pushFile(files, parent, wideOld + MI_PATH("-hd.png"), wideName + MI_PATH("-hd.png"));
            pushFile(files, parent, wideOld + MI_PATH(".png"), wideName + MI_PATH(".png"));
            pushFile(files, parent, wideOld + MI_PATH("-uhd.plist"), wideName + MI_PATH("-uhd.plist"));
            pushFile(files, parent, wideOld + MI_PATH("-hd.plist"), wideName + MI_PATH("-hd.plist"));
            pushFile(files, parent, wideOld + MI_PATH(".plist"), wideName + MI_PATH(".plist"));
        }

        if (files.size() > 0) {
            fmt::format_to(std::back_inserter(message),
                "\n<cr>This will overwrite the following files:</c>{}\n<cr>These cannot be restored!</c>", fmt::to_string(files));
        }

        createQuickPopup(fmt::format("Rename {}", unlockName).c_str(), fmt::to_string(message), "No", "Yes", [
            this, info, old = std::move(old), name = std::move(name), wideOld = std::move(wideOld), wideName = std::move(wideName), popup
        ](auto, bool btn2) {
            if (!btn2) return;

            auto type = info->getType();
            auto parent = info->getTexture().parent_path();
            if (type >= IconType::DeathEffect) {
                parent = parent.parent_path();
                renameFile(parent, wideOld, wideName);
            }
            else if (type <= IconType::Jetpack) {
                renameFile(parent, wideOld + MI_PATH("-uhd.png"), wideName + MI_PATH("-uhd.png"));
                renameFile(parent, wideOld + MI_PATH("-hd.png"), wideName + MI_PATH("-hd.png"));
                renameFile(parent, wideOld + MI_PATH(".png"), wideName + MI_PATH(".png"));
                renameFile(parent, wideOld + MI_PATH("-uhd.plist"), wideName + MI_PATH("-uhd.plist"));
                renameFile(parent, wideOld + MI_PATH("-hd.plist"), wideName + MI_PATH("-hd.plist"));
                renameFile(parent, wideOld + MI_PATH(".plist"), wideName + MI_PATH(".plist"));
            }

            Popup::onClose(nullptr);
            popup->close();
            more_icons::renameIcon(info, name);
            MoreIcons::notifySuccess("{} renamed to {}!", old, name);
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
    if (name.empty() || name == m_info->getShortName()) return Popup::onClose(sender);

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
