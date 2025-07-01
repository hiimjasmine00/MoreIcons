#include "IconNamePopup.hpp"
#include "EditIconPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

IconNamePopup* IconNamePopup::create(IconInfo* info) {
    auto ret = new IconNamePopup();
    if (ret->initAnchored(350.0f, 130.0f, info, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

Result<> replaceFile(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) {
    std::error_code code;
    if (!MoreIcons::doesExist(oldPath)) return Ok();
    if (MoreIcons::doesExist(newPath)) {
        if (!std::filesystem::remove(newPath, code)) return Err("Failed to remove {}: {}", newPath.filename(), code.message());
    }
    std::filesystem::rename(oldPath, newPath, code);
    if (code) return Err("Failed to rename {}: {}", oldPath.filename(), code.message());
    else return Ok();
}

bool IconNamePopup::setup(IconInfo* info) {
    auto unlock = (int)GameManager::get()->iconTypeToUnlockType(info->type);
    auto unlockName = EditIconPopup::uppercase[unlock];

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

    auto confirmButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Confirm", "goldFont.fnt", "GJ_button_05.png", 0.8f), [
        this, info, unlockName
    ](auto) {
        auto name = m_nameInput->getString();
        if (name.empty()) return Notification::create("Name cannot be empty.", NotificationIcon::Info)->show();

        auto& old = info->shortName;
        if (name == old) return Notification::create(fmt::format("Name is already set to {}.", name), NotificationIcon::Info)->show();

        auto message = fmt::format("Are you sure you want to rename <cy>{}</c> to <cy>{}</c>?", old, name);

        auto parent = std::filesystem::path(info->textures[0]).parent_path();
        std::string files;

        if (info->type == IconType::Special) {
            if (MoreIcons::doesExist(parent / (old + ".png")) && MoreIcons::doesExist(parent / (name + ".png")))
                files += fmt::format("\n<cg>{}.png</c>", name);

            if (MoreIcons::doesExist(parent / (old + ".json")) && MoreIcons::doesExist(parent / (name + ".json")))
                files += fmt::format("\n<cg>{}.json</c>", name);
        }
        else if (info->type <= IconType::Jetpack) {
            if (MoreIcons::doesExist(parent / (old + "-uhd.png")) && MoreIcons::doesExist(parent / (name + "-uhd.png")))
                files += fmt::format("\n<cg>{}-uhd.png</c>", name);

            if (MoreIcons::doesExist(parent / (old + "-hd.png")) && MoreIcons::doesExist(parent / (name + "-hd.png")))
                files += fmt::format("\n<cg>{}-hd.png</c>", name);

            if (MoreIcons::doesExist(parent / (old + ".png")) && MoreIcons::doesExist(parent / (name + ".png")))
                files += fmt::format("\n<cg>{}.png</c>", name);

            if (MoreIcons::doesExist(parent / (old + "-uhd.plist")) && MoreIcons::doesExist(parent / (name + "-uhd.plist")))
                files += fmt::format("\n<cg>{}-uhd.plist</c>", name);

            if (MoreIcons::doesExist(parent / (old + "-hd.plist")) && MoreIcons::doesExist(parent / (name + "-hd.plist")))
                files += fmt::format("\n<cg>{}-hd.plist</c>", name);

            if (MoreIcons::doesExist(parent / (old + ".plist")) && MoreIcons::doesExist(parent / (name + ".plist")))
                files += fmt::format("\n<cg>{}.plist</c>", name);
        }

        if (!files.empty()) message += "\n<cr>This will overwrite the following files:</c>" + files + "\n<cr>These cannot be restored!</c>";

        createQuickPopup(fmt::format("Rename {}", unlockName).c_str(), message, "No", "Yes", [this, info, old, name](auto, bool btn2) {
            if (!btn2) return;

            auto parent = std::filesystem::path(info->textures[0]).parent_path();

            if (info->type == IconType::Special) {
                if (GEODE_UNWRAP_IF_ERR(err, replaceFile(parent / (old + ".png"), parent / (name + ".png"))))
                    return Notification::create(fmt::format("Failed to rename {}.png: {}", old, err), NotificationIcon::Error)->show();
                if (GEODE_UNWRAP_IF_ERR(err, replaceFile(parent / (old + ".json"), parent / (name + ".json"))))
                    return Notification::create(fmt::format("Failed to rename {}.json: {}", old, err), NotificationIcon::Error)->show();
            }
            else if (info->type <= IconType::Jetpack) {
                if (GEODE_UNWRAP_IF_ERR(err, replaceFile(parent / (old + "-uhd.png"), parent / (name + "-uhd.png"))))
                    return Notification::create(fmt::format("Failed to rename {}-uhd.png: {}", old, err), NotificationIcon::Error)->show();
                if (GEODE_UNWRAP_IF_ERR(err, replaceFile(parent / (old + "-hd.png"), parent / (name + "-hd.png"))))
                    return Notification::create(fmt::format("Failed to rename {}-hd.png: {}", old, err), NotificationIcon::Error)->show();
                if (GEODE_UNWRAP_IF_ERR(err, replaceFile(parent / (old + ".png"), parent / (name + ".png"))))
                    return Notification::create(fmt::format("Failed to rename {}.png: {}", old, err), NotificationIcon::Error)->show();
                if (GEODE_UNWRAP_IF_ERR(err, replaceFile(parent / (old + "-uhd.plist"), parent / (name + "-uhd.plist"))))
                    return Notification::create(fmt::format("Failed to rename {}-uhd.plist: {}", old, err), NotificationIcon::Error)->show();
                if (GEODE_UNWRAP_IF_ERR(err, replaceFile(parent / (old + "-hd.plist"), parent / (name + "-hd.plist"))))
                    return Notification::create(fmt::format("Failed to rename {}-hd.plist: {}", old, err), NotificationIcon::Error)->show();
                if (GEODE_UNWRAP_IF_ERR(err, replaceFile(parent / (old + ".plist"), parent / (name + ".plist"))))
                    return Notification::create(fmt::format("Failed to rename {}.plist: {}", old, err), NotificationIcon::Error)->show();
            }

            m_nameInput->setString("");
            onClose(nullptr);
            auto editIconPopup = CCScene::get()->getChildByType<EditIconPopup>(0);
            if (editIconPopup) editIconPopup->fullClose();
            MoreIconsAPI::renameIcon(info, name);
            Notification::create(fmt::format("{} renamed to {}!", old, name), NotificationIcon::Success)->show();
            if (editIconPopup) editIconPopup->updateGarage();
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

    auto unlock = (int)GameManager::get()->iconTypeToUnlockType(m_iconType);
    createQuickPopup(
        fmt::format("Exit {} Name Editor", EditIconPopup::uppercase[unlock]).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} name editor</c>?\n<cr>All unsaved changes will be lost!</c>",
            EditIconPopup::lowercase[unlock]),
        "No",
        "Yes",
        [this, sender](auto, bool btn2) {
            if (btn2) Popup::onClose(sender);
        }
    );
}
