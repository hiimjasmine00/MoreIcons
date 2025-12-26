#include "IconNamePopup.hpp"
#include "MoreInfoPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Filesystem.hpp"
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

void pushFile(fmt::memory_buffer& buffer, const std::filesystem::path& file1, const std::filesystem::path& file2, std::string_view name) {
    if (Filesystem::doesExist(file1) && Filesystem::doesExist(file2)) {
        fmt::vformat_to(std::back_inserter(buffer), "{}", fmt::make_format_args(name));
    }
}

bool renameFile(const std::filesystem::path& file1, const std::filesystem::path& file2) {
    auto res = Filesystem::renameFile(file1, file2);
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

        auto parent = Filesystem::parentPath(info->getTexture());
        auto type = info->getType();
        if (type >= IconType::DeathEffect) parent = Filesystem::parentPath(std::move(parent));

        auto stemOld = parent / Filesystem::strWide(old);
        auto stemNew = parent / Filesystem::strWide(name);

        fmt::memory_buffer files;
        if (type >= IconType::DeathEffect) {
            pushFile(files, stemOld, stemNew, fmt::format("\n<cg>{}</c>", old));
        }
        else if (type <= IconType::Jetpack) {
            pushFile(files, fmt::format(L("{}-uhd.png"), stemOld), fmt::format(L("{}-uhd.png"), stemNew),
                fmt::format("\n<cg>{}-uhd.png</c>", old));
            pushFile(files, fmt::format(L("{}-hd.png"), stemOld), fmt::format(L("{}-hd.png"), stemNew),
                fmt::format("\n<cg>{}-hd.png</c>", old));
            pushFile(files, fmt::format(L("{}.png"), stemOld), fmt::format(L("{}.png"), stemNew),
                fmt::format("\n<cg>{}.png</c>", old));
            pushFile(files, fmt::format(L("{}-uhd.plist"), stemOld), fmt::format(L("{}-uhd.plist"), stemNew),
                fmt::format("\n<cg>{}-uhd.plist</c>", old));
            pushFile(files, fmt::format(L("{}-hd.plist"), stemOld), fmt::format(L("{}-hd.plist"), stemNew),
                fmt::format("\n<cg>{}-hd.plist</c>", old));
            pushFile(files, fmt::format(L("{}.plist"), stemOld), fmt::format(L("{}.plist"), stemNew),
                fmt::format("\n<cg>{}.plist</c>", old));
        }

        if (files.size() > 0) {
            fmt::format_to(std::back_inserter(message),
                "\n<cr>This will overwrite the following files:</c>{}\n<cr>These cannot be restored!</c>", fmt::to_string(files));
        }

        createQuickPopup(fmt::format("Rename {}", unlockName).c_str(), fmt::to_string(message), "No", "Yes", [
            this, info, parent = std::move(parent), old = std::move(old), name = std::move(name),
            stemOld = std::move(stemOld), stemNew = std::move(stemNew), popup
        ](auto, bool btn2) {
            if (!btn2) return;

            auto type = info->getType();
            if (type >= IconType::DeathEffect) {
                if (!renameFile(stemOld, stemNew)) return;
            }
            else if (type <= IconType::Jetpack) {
                if (!renameFile(fmt::format(L("{}-uhd.png"), stemOld), fmt::format(L("{}-uhd.png"), stemNew))) return;
                if (!renameFile(fmt::format(L("{}-hd.png"), stemOld), fmt::format(L("{}-hd.png"), stemNew))) return;
                if (!renameFile(fmt::format(L("{}.png"), stemOld), fmt::format(L("{}.png"), stemNew))) return;
                if (!renameFile(fmt::format(L("{}-uhd.plist"), stemOld), fmt::format(L("{}-uhd.plist"), stemNew))) return;
                if (!renameFile(fmt::format(L("{}-hd.plist"), stemOld), fmt::format(L("{}-hd.plist"), stemNew))) return;
                if (!renameFile(fmt::format(L("{}.plist"), stemOld), fmt::format(L("{}.plist"), stemNew))) return;
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
