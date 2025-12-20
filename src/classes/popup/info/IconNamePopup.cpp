#define FMT_CPP_LIB_FILESYSTEM 0
#include "IconNamePopup.hpp"
#include "MoreInfoPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
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

void pushFile(
    fmt::memory_buffer& buffer, const std::filesystem::path& parent, const std::filesystem::path& file1, const std::filesystem::path& file2
) {
    if (MoreIcons::doesExist(parent / file1) && MoreIcons::doesExist(parent / file2)) {
        fmt::format_to(std::back_inserter(buffer), "\n<cg>{}</c>", file1);
    }
}

bool renameFile(const std::filesystem::path& parent, const std::filesystem::path& from, const std::filesystem::path& to) {
    auto res = MoreIcons::renameFile(parent / from, parent / to);
    if (res.isErr()) Notify::error(res.unwrapErr());
    return res.isOk();
}

bool IconNamePopup::setup(MoreInfoPopup* popup, IconInfo* info) {
    m_iconType = info->getType();
    auto unlockName = Constants::getIconLabel(m_iconType, true, false);

    setID("IconNamePopup");
    setTitle(fmt::format("Edit {} Name", unlockName));
    m_title->setID("icon-name-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

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
        if (name.empty()) return Notify::info("Name cannot be empty.");

        auto old = info->getShortName();
        if (name == old) return Notify::info("Name is already set to {}.", name);

        fmt::memory_buffer message;
        fmt::format_to(std::back_inserter(message), "Are you sure you want to rename <cy>{}</c> to <cy>{}</c>?", old, name);

        auto parent = info->getTexture().parent_path();
        fmt::memory_buffer files;

        auto wideOld = MoreIcons::strWide(old);
        auto wideName = MoreIcons::strWide(name);
        auto type = info->getType();
        if (type >= IconType::DeathEffect) {
            parent = parent.parent_path();
            pushFile(files, parent, wideOld, wideName);
        }
        else if (type <= IconType::Jetpack) {
            pushFile(files, parent, fmt::format(L("{}-uhd.png"), wideOld), fmt::format(L("{}-uhd.png"), wideName));
            pushFile(files, parent, fmt::format(L("{}-hd.png"), wideOld), fmt::format(L("{}-hd.png"), wideName));
            pushFile(files, parent, fmt::format(L("{}.png"), wideOld), fmt::format(L("{}.png"), wideName));
            pushFile(files, parent, fmt::format(L("{}-uhd.plist"), wideOld), fmt::format(L("{}-uhd.plist"), wideName));
            pushFile(files, parent, fmt::format(L("{}-hd.plist"), wideOld), fmt::format(L("{}-hd.plist"), wideName));
            pushFile(files, parent, fmt::format(L("{}.plist"), wideOld), fmt::format(L("{}.plist"), wideName));
        }

        if (files.size() > 0) {
            fmt::format_to(std::back_inserter(message),
                "\n<cr>This will overwrite the following files:</c>{}\n<cr>These cannot be restored!</c>", fmt::to_string(files));
        }

        createQuickPopup(fmt::format("Rename {}", unlockName).c_str(), fmt::to_string(message), "No", "Yes", [
            this, info, parent = std::move(parent), old = std::move(old), name = std::move(name),
            wideOld = std::move(wideOld), wideName = std::move(wideName), popup
        ](auto, bool btn2) {
            if (!btn2) return;

            auto type = info->getType();
            if (type >= IconType::DeathEffect) {
                if (!renameFile(parent, wideOld, wideName)) return;
            }
            else if (type <= IconType::Jetpack) {
                if (!renameFile(parent, fmt::format(L("{}-uhd.png"), wideOld), fmt::format(L("{}-uhd.png"), wideName))) return;
                if (!renameFile(parent, fmt::format(L("{}-hd.png"), wideOld), fmt::format(L("{}-hd.png"), wideName))) return;
                if (!renameFile(parent, fmt::format(L("{}.png"), wideOld), fmt::format(L("{}.png"), wideName))) return;
                if (!renameFile(parent, fmt::format(L("{}-uhd.plist"), wideOld), fmt::format(L("{}-uhd.plist"), wideName))) return;
                if (!renameFile(parent, fmt::format(L("{}-hd.plist"), wideOld), fmt::format(L("{}-hd.plist"), wideName))) return;
                if (!renameFile(parent, fmt::format(L("{}.plist"), wideOld), fmt::format(L("{}.plist"), wideName))) return;
            }

            Popup::onClose(nullptr);
            popup->close();
            more_icons::renameIcon(info, name);
            Notify::success("{} renamed to {}!", old, name);
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

    auto type = m_iconType;
    createQuickPopup(
        fmt::format("Exit {} Name Editor", Constants::getIconLabel(type, true, false)).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} name editor</c>?", Constants::getIconLabel(type, false, false)),
        "No",
        "Yes",
        [this, sender](auto, bool btn2) {
            if (btn2) Popup::onClose(sender);
        }
    );
}
