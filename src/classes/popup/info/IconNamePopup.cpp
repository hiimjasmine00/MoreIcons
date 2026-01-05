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
    if (ret->init(popup, info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IconNamePopup::init(MoreInfoPopup* popup, IconInfo* info) {
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
    auto name = MoreIcons::getText(m_nameInput);
    if (name.empty()) return Notify::info("Name cannot be empty.");

    std::string_view old = m_info->getShortName();
    if (name == old) return Notify::info("Name is already set to {}.", name);

    fmt::memory_buffer message;
    fmt::format_to(std::back_inserter(message), "Are you sure you want to rename <cy>{}</c> to <cy>{}</c>?", old, name);

    auto parent = Filesystem::parentPath(m_info->getTexture());
    auto type = m_iconType;
    if (type >= IconType::DeathEffect) parent = Filesystem::parentPath(std::move(parent));

    auto stemOld = parent / Filesystem::strWide(old);
    auto stemNew = parent / Filesystem::strWide(name);

    m_pendingPaths.clear();

    fmt::memory_buffer files;
    if (type >= IconType::DeathEffect) {
        if (Filesystem::doesExist(stemOld)) {
            if (Filesystem::doesExist(stemNew)) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(stemOld), std::move(stemNew));
        }
    }
    else {
        std::filesystem::path uhdPng = fmt::format(L("{}-uhd.png"), stemOld);
        if (Filesystem::doesExist(uhdPng)) {
            std::filesystem::path newPath = fmt::format(L("{}-uhd.png"), stemNew);
            if (Filesystem::doesExist(newPath)) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-uhd.png</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(uhdPng), std::move(newPath));
        }

        std::filesystem::path hdPng = fmt::format(L("{}-hd.png"), stemOld);
        if (Filesystem::doesExist(hdPng)) {
            std::filesystem::path newPath = fmt::format(L("{}-hd.png"), stemNew);
            if (Filesystem::doesExist(newPath)) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-hd.png</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(hdPng), std::move(newPath));
        }

        std::filesystem::path png = fmt::format(L("{}.png"), stemOld);
        if (Filesystem::doesExist(png)) {
            std::filesystem::path newPath = fmt::format(L("{}.png"), stemNew);
            if (Filesystem::doesExist(newPath)) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}.png</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(png), std::move(newPath));
        }

        std::filesystem::path uhdPlist = fmt::format(L("{}-uhd.plist"), stemOld);
        if (Filesystem::doesExist(uhdPlist)) {
            std::filesystem::path newPath = fmt::format(L("{}-uhd.plist"), stemNew);
            if (Filesystem::doesExist(newPath)) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-uhd.plist</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(uhdPlist), std::move(newPath));
        }

        std::filesystem::path hdPlist = fmt::format(L("{}-hd.plist"), stemOld);
        if (Filesystem::doesExist(hdPlist)) {
            std::filesystem::path newPath = fmt::format(L("{}-hd.plist"), stemNew);
            if (Filesystem::doesExist(newPath)) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}-hd.plist</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(hdPlist), std::move(newPath));
        }

        std::filesystem::path plist = fmt::format(L("{}.plist"), stemOld);
        if (Filesystem::doesExist(plist)) {
            std::filesystem::path newPath = fmt::format(L("{}.plist"), stemNew);
            if (Filesystem::doesExist(newPath)) {
                fmt::format_to(std::back_inserter(files), "\n<cg>{}.plist</c>", old);
            }
            m_pendingPaths.emplace_back(std::move(plist), std::move(newPath));
        }
    }

    if (files.size() > 0) {
        fmt::format_to(std::back_inserter(message),
            "\n<cr>This will overwrite the following files:</c>{}\n<cr>These cannot be restored!</c>", fmt::to_string(files));
    }

    auto alert = FLAlertLayer::create(
        this,
        fmt::format("Rename {}", Constants::getSingularUppercase(type)).c_str(),
        fmt::to_string(message),
        "No",
        "Yes",
        350.0f
    );
    alert->setTag(0);
    alert->show();
}

void IconNamePopup::onClose(CCObject* sender) {
    auto name = MoreIcons::getText(m_nameInput);
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

            auto name = MoreIcons::getText(m_nameInput);
            auto notif = fmt::format("{} renamed to {}!", m_info->getShortName(), name);
            more_icons::renameIcon(m_info, std::string(name));

            close();
            m_parentPopup->close();

            Notify::success(notif);
            MoreIcons::updateGarage();
            break;
        }
        case 1: {
            close();
            break;
        }
    }
}
