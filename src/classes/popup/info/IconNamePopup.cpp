#include "IconNamePopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/utils/StringBuffer.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

IconNamePopup* IconNamePopup::create(IconInfo* info) {
    auto ret = new IconNamePopup();
    if (ret->init(info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IconNamePopup::init(IconInfo* info) {
    if (!BasePopup::init(350.0f, 130.0f)) return false;

    auto type = info->getType();

    setID("IconNamePopup");
    setTitle(fmt::format("Edit {} Name", Constants::getSingularUppercase(type)));
    m_title->setID("icon-name-title");

    m_info = info;
    m_iconType = type;

    m_nameInput = TextInput::create(300.0f, "Name");
    m_nameInput->setPosition({ 175.0f, 70.0f });
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_nameInput->setMaxCharCount(100);
    m_nameInput->setString(info->getShortName());
    m_nameInput->setID("name-input");
    m_mainLayer->addChild(m_nameInput);

    auto confirmButton = CCMenuItemSpriteExtra::create(ButtonSprite::create("Confirm", 0.8f), this, menu_selector(IconNamePopup::onConfirm));
    confirmButton->setPosition({ 175.0f, 30.0f });
    confirmButton->setID("confirm-button");
    m_buttonMenu->addChild(confirmButton);

    handleTouchPriority(this);

    return true;
}

void IconNamePopup::onConfirm(CCObject* sender) {
    auto& name = m_nameInput->getString();
    if (name.empty()) return Notify::info("Name cannot be empty.");

    std::string_view old = m_info->getShortName();
    if (name == old) return Notify::info("Name is already set to {}.", name);

    StringBuffer message;
    message.append("Are you sure you want to rename <cy>{}</c> to <cy>{}</c>?", old, name);

    auto parent = m_info->getTexture().parent().value_or("").toPath();
    auto type = m_iconType;
    if (type >= IconType::DeathEffect) parent = parent.parent().value_or("");

    auto stemOld = parent / old;
    auto stemNew = parent / name;

    m_pendingPaths.clear();

    StringBuffer files;
    if (type >= IconType::DeathEffect) {
        if (stemOld.exists()) {
            if (stemNew.exists()) {
                files.append("\n<cg>{}</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(stemOld), std::move(stemNew));
        }
    }
    else {
        asp::fs::Path uhdPng = fmt::format("{}-uhd.png", stemOld);
        if (uhdPng.exists()) {
            asp::fs::Path newPath = fmt::format("{}-uhd.png", stemNew);
            if (newPath.exists()) {
                files.append("\n<cg>{}-uhd.png</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(uhdPng), std::move(newPath));
        }

        asp::fs::Path hdPng = fmt::format("{}-hd.png", stemOld);
        if (hdPng.exists()) {
            asp::fs::Path newPath = fmt::format("{}-hd.png", stemNew);
            if (newPath.exists()) {
                files.append("\n<cg>{}-hd.png</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(hdPng), std::move(newPath));
        }

        asp::fs::Path png = fmt::format("{}.png", stemOld);
        if (png.exists()) {
            asp::fs::Path newPath = fmt::format("{}.png", stemNew);
            if (newPath.exists()) {
                files.append("\n<cg>{}.png</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(png), std::move(newPath));
        }

        asp::fs::Path uhdPlist = fmt::format("{}-uhd.plist", stemOld);
        if (uhdPlist.exists()) {
            asp::fs::Path newPath = fmt::format("{}-uhd.plist", stemNew);
            if (newPath.exists()) {
                files.append("\n<cg>{}-uhd.plist</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(uhdPlist), std::move(newPath));
        }

        asp::fs::Path hdPlist = fmt::format("{}-hd.plist", stemOld);
        if (hdPlist.exists()) {
            asp::fs::Path newPath = fmt::format("{}-hd.plist", stemNew);
            if (newPath.exists()) {
                files.append("\n<cg>{}-hd.plist</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(hdPlist), std::move(newPath));
        }

        asp::fs::Path plist = fmt::format("{}.plist", stemOld);
        if (plist.exists()) {
            asp::fs::Path newPath = fmt::format("{}.plist", stemNew);
            if (newPath.exists()) {
                files.append("\n<cg>{}.plist</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(plist), std::move(newPath));
        }
    }

    if (files.size() > 0) {
        message.append("\n<cr>This will overwrite the following files:</c>{}\n<cr>These cannot be restored!</c>", files);
    }

    auto alert = FLAlertLayer::create(
        this,
        fmt::format("Rename {}", Constants::getSingularUppercase(type)).c_str(),
        message.str(),
        "No",
        "Yes",
        350.0f
    );
    alert->setTag(0);
    alert->show();
}

void IconNamePopup::onClose(CCObject* sender) {
    auto& name = m_nameInput->getString();
    if (name.empty() || name == m_info->getShortName()) return close();

    auto type = m_iconType;
    auto alert = FLAlertLayer::create(
        this,
        fmt::format("Exit {} Name Editor", Constants::getSingularUppercase(type)).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} name editor</c>?", Constants::getSingularLowercase(type)),
        "No",
        "Yes",
        350.0f
    );
    alert->setTag(1);
    alert->show();
}

void IconNamePopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (!btn2) return;
    switch (layer->getTag()) {
        case 0: {
            for (auto& [oldPath, newPath] : m_pendingPaths) {
                auto res = Filesystem::renameFile(oldPath, newPath);
                if (res.isErr()) return Notify::error(res.unwrapErr());
            }

            auto& name = m_nameInput->getString();
            auto notif = fmt::format("{} renamed to {}!", m_info->getShortName(), name);
            more_icons::renameIcon(m_info, name);
            MoreIcons::updateGarageAndNotify(notif);
            break;
        }
        case 1: {
            close();
            break;
        }
    }
}
