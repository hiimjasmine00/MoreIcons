#include "SaveIconPopup.hpp"
#include "ImageRenderer.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Icons.hpp"
#include "../../../utils/Load.hpp"
#include "../../../utils/Notify.hpp"
#include <algorithm>
#include <Geode/binding/ButtonSprite.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

SaveIconPopup* SaveIconPopup::create(
    BasePopup* popup1, BasePopup* popup2, IconType type,
    const std::unordered_map<std::string, FrameDefinition>& definitions, const std::unordered_map<std::string, Ref<CCSpriteFrame>>& frames
) {
    auto ret = new SaveIconPopup();
    if (ret->init(popup1, popup2, type, definitions, frames)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SaveIconPopup::init(
    BasePopup* popup1, BasePopup* popup2, IconType type,
    const std::unordered_map<std::string, FrameDefinition>& definitions, const std::unordered_map<std::string, Ref<CCSpriteFrame>>& frames
) {
    if (!BasePopup::init(350.0f, 130.0f, "geode.loader/GE_square03.png")) return false;

    setID("SaveIconPopup");
    setTitle(fmt::format("Save {}", Constants::getSingularUppercase(type)));
    m_title->setID("save-icon-title");

    m_parentPopup1 = popup1;
    m_parentPopup2 = popup2;
    m_iconType = type;
    m_definitions = &definitions;
    m_frames = &frames;

    m_nameInput = TextInput::create(300.0f, "Icon Name");
    m_nameInput->setPosition({ 175.0f, 70.0f });
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_nameInput->setMaxCharCount(100);
    m_nameInput->setID("name-input");
    m_mainLayer->addChild(m_nameInput);

    auto saveButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f), this, menu_selector(SaveIconPopup::onSave)
    );
    saveButton->setPosition({ 175.0f, 30.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}

void SaveIconPopup::onSave(CCObject* sender) {
    auto iconName = MoreIcons::getText(m_nameInput);
    if (iconName.empty()) return Notify::info("Please enter a name.");

    auto stem = std::move(const_cast<std::filesystem::path::string_type&>(MoreIcons::getIconStem(iconName, m_iconType).native()));
    m_pngs[0] = fmt::format(L("{}-uhd.png"), stem);
    m_pngs[1] = fmt::format(L("{}-hd.png"), stem);
    m_pngs[2] = fmt::format(L("{}.png"), stem);
    m_plists[0] = fmt::format(L("{}-uhd.plist"), stem);
    m_plists[1] = fmt::format(L("{}-hd.plist"), stem);
    m_plists[2] = fmt::format(L("{}.plist"), stem);
    if (std::ranges::any_of(m_plists, [](const std::filesystem::path& path) {
        return Filesystem::doesExist(path);
    }) || std::ranges::any_of(m_pngs, [](const std::filesystem::path& path) {
        return Filesystem::doesExist(path);
    })) {
        auto alert = FLAlertLayer::create(
            this,
            "Existing Icon",
            fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", iconName),
            "No",
            "Yes",
            350.0f
        );
        alert->setTag(0);
        alert->show();
    }
    else saveIcon();
}

void SaveIconPopup::saveIcon() {
    auto name = MoreIcons::getText(m_nameInput);

    std::array<texpack::Packer, 3> packers = {};
    auto scaleFactor = Get::Director()->getContentScaleFactor();
    int index;
    if (scaleFactor >= 4.0f) index = 0;
    else if (scaleFactor >= 2.0f) index = 1;
    else index = 2;
    std::array scales = { 4.0f / scaleFactor, 2.0f / scaleFactor, 1.0f / scaleFactor };
    for (auto& [frameName, frameRef] : *m_frames) {
        auto it = m_definitions->find(frameName);
        if (it == m_definitions->end()) continue;

        auto& definition = it->second;
        auto joinedName = fmt::format("{}{}.png", name, frameName);
        auto frame = frameRef.data();
        for (int i = 0; i < 3; i++) {
            auto node = CCNode::create();
            node->setScale(scales[i]);
            node->setAnchorPoint({ 0.0f, 0.0f });
            auto sprite = CCSprite::createWithSpriteFrame(frame);
            sprite->setPosition({ definition.offsetX, definition.offsetY });
            sprite->setScaleX(definition.scaleX);
            sprite->setScaleY(definition.scaleY);
            sprite->setRotationX(definition.rotationX);
            sprite->setRotationY(definition.rotationY);
            node->addChild(sprite);
            auto boundingSize = sprite->boundingBox().size;
            node->setContentSize(boundingSize + CCSize { std::abs(definition.offsetX * 2.0f), std::abs(definition.offsetY * 2.0f) });
            sprite->setPosition(node->getContentSize() / 2.0f + sprite->getPosition());
            sprite->setBlendFunc({ GL_ONE, GL_ZERO });
            packers[i].frame(joinedName, ImageRenderer::getImage(node));
            node->release();
            sprite->release();
        }
    }

    if (auto res = ImageRenderer::save(packers[0], m_pngs[0], m_plists[0], fmt::format("icons/{}-uhd.png", name)); res.isErr()) {
        return Notify::error("Failed to save UHD icon: {}", res.unwrapErr());
    }

    if (auto res = ImageRenderer::save(packers[1], m_pngs[1], m_plists[1], fmt::format("icons/{}-hd.png", name)); res.isErr()) {
        return Notify::error("Failed to save HD icon: {}", res.unwrapErr());
    }

    if (auto res = ImageRenderer::save(packers[2], m_pngs[2], m_plists[2], fmt::format("icons/{}.png", name)); res.isErr()) {
        return Notify::error("Failed to save SD icon: {}", res.unwrapErr());
    }

    auto type = m_iconType;
    if (auto icon = more_icons::getIcon(name, type)) {
        more_icons::updateIcon(icon);
    }
    else {
        icon = more_icons::addIcon(std::string(name), std::string(name), type,
            std::move(m_pngs[index]), std::move(m_plists[index]), Get::Director()->getLoadedTextureQuality());
        if (Icons::preloadIcons) Icons::createAndAddFrames(icon);
    }

    auto notif = fmt::format("{} saved!", name);

    close();
    m_parentPopup2->close();
    m_parentPopup1->close();

    Notify::success(notif);
    MoreIcons::updateGarage();
}

void SaveIconPopup::onClose(CCObject* sender) {
    if (MoreIcons::getText(m_nameInput).empty()) return close();

    auto type = m_iconType;
    auto alert = FLAlertLayer::create(
        this,
        fmt::format("Exit {} Saver", Constants::getSingularUppercase(type)).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} saver</c>?", Constants::getSingularLowercase(type)),
        "No",
        "Yes",
        350.0f
    );
    alert->setTag(1);
    alert->show();
}

void SaveIconPopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (!btn2) return;
    switch (layer->getTag()) {
        case 0: saveIcon(); break;
        case 1: close(); break;
    }
}
