#include "SaveIconPopup.hpp"
#include "ImageRenderer.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Icons.hpp"
#include "../../../utils/Load.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

SaveIconPopup* SaveIconPopup::create(
    BasePopup* popup1, BasePopup* popup2, IconType type,
    const StringMap<FrameDefinition>& definitions, const StringMap<geode::Ref<cocos2d::CCSpriteFrame>>& frames
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
    const StringMap<FrameDefinition>& definitions, const StringMap<geode::Ref<cocos2d::CCSpriteFrame>>& frames
) {
    if (!BasePopup::init(350.0f, 130.0f, "geode.loader/GE_square03.png")) return false;

    setID("SaveIconPopup");
    setTitle(fmt::format("Save {}", Constants::getSingularUppercase(type)));
    m_title->setID("save-icon-title");

    m_parentPopup1 = popup1;
    m_parentPopup2 = popup2;
    m_iconType = type;
    m_definitions = definitions;
    m_frames = frames;

    m_nameInput = TextInput::create(300.0f, "Icon Name");
    m_nameInput->setPosition({ 175.0f, 70.0f });
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_nameInput->setMaxCharCount(100);
    m_nameInput->setID("name-input");
    m_mainLayer->addChild(m_nameInput);

    auto saveSprite = ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto saveButton = CCMenuItemExt::createSpriteExtra(saveSprite, [this](auto) {
        auto iconName = m_nameInput->getString();
        if (iconName.empty()) return Notify::info("Please enter a name.");

        auto stem = std::move(const_cast<std::filesystem::path::string_type&>(MoreIcons::getIconStem(iconName, m_iconType).native()));
        if (
            Filesystem::doesExist(fmt::format(L("{}-uhd.plist"), stem)) ||
            Filesystem::doesExist(fmt::format(L("{}-hd.plist"), stem)) ||
            Filesystem::doesExist(fmt::format(L("{}.plist"), stem)) ||
            Filesystem::doesExist(fmt::format(L("{}-uhd.png"), stem)) ||
            Filesystem::doesExist(fmt::format(L("{}-hd.png"), stem)) ||
            Filesystem::doesExist(fmt::format(L("{}.png"), stem))
        ) {
            createQuickPopup(
                "Existing Icon",
                fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", iconName),
                "No",
                "Yes",
                [this, stem = std::move(stem)](auto, bool btn2) {
                    if (btn2) saveIcon(stem);
                }
            );
        }
        else saveIcon(stem);
    });
    saveButton->setPosition({ 175.0f, 30.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}

bool SaveIconPopup::checkFrame(std::string_view suffix) {
    auto ret = m_frames.contains(suffix);
    if (!ret) Notify::info("Missing {}{}.", m_nameInput->getString(), suffix);
    return ret;
}

void SaveIconPopup::saveIcon(Filesystem::PathView stem) {
    auto type = m_iconType;
    if (type == IconType::Robot || type == IconType::Spider) {
        if (!checkFrame("_01_001") || !checkFrame("_01_2_001") || !checkFrame("_01_glow_001")) return;
        if (!checkFrame("_02_001") || !checkFrame("_02_2_001") || !checkFrame("_02_glow_001")) return;
        if (!checkFrame("_03_001") || !checkFrame("_03_2_001") || !checkFrame("_03_glow_001")) return;
        if (!checkFrame("_04_001") || !checkFrame("_04_2_001") || !checkFrame("_04_glow_001")) return;
    }
    else {
        if (!checkFrame("_001")) return;
        if (!checkFrame("_2_001")) return;
        if (type == IconType::Ufo && !checkFrame("_3_001")) return;
        if (!checkFrame("_glow_001")) return;
    }

    auto name = m_nameInput->getString();

    std::array<texpack::Packer, 3> packers = {};
    auto scaleFactor = Get::Director()->getContentScaleFactor();
    std::array scales = { 4.0f / scaleFactor, 2.0f / scaleFactor, 1.0f / scaleFactor };
    for (auto& [frameName, frame] : m_frames) {
        auto& definition = m_definitions[frameName];
        auto joinedName = fmt::format("{}{}.png", name, frameName);
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

    std::filesystem::path selectedPNG;
    std::filesystem::path selectedPlist;

    std::filesystem::path uhdPng = fmt::format(L("{}-uhd.png"), stem);
    std::filesystem::path uhdPlist = fmt::format(L("{}-uhd.plist"), stem);
    if (auto res = ImageRenderer::save(packers[0], uhdPng, uhdPlist, fmt::format("icons/{}-uhd.png", name)); res.isErr()) {
        return Notify::error("Failed to save UHD icon: {}", res.unwrapErr());
    }
    if (scales[0] == 1.0f) {
        selectedPNG = std::move(uhdPng);
        selectedPlist = std::move(uhdPlist);
    }

    std::filesystem::path hdPng = fmt::format(L("{}-hd.png"), stem);
    std::filesystem::path hdPlist = fmt::format(L("{}-hd.plist"), stem);
    if (auto res = ImageRenderer::save(packers[1], hdPng, hdPlist, fmt::format("icons/{}-hd.png", name)); res.isErr()) {
        return Notify::error("Failed to save HD icon: {}", res.unwrapErr());
    }
    if (scales[1] == 1.0f) {
        selectedPNG = std::move(hdPng);
        selectedPlist = std::move(hdPlist);
    }

    std::filesystem::path sdPng = fmt::format(L("{}.png"), stem);
    std::filesystem::path sdPlist = fmt::format(L("{}.plist"), stem);
    if (auto res = ImageRenderer::save(packers[2], sdPng, sdPlist, fmt::format("icons/{}.png", name)); res.isErr()) {
        return Notify::error("Failed to save SD icon: {}", res.unwrapErr());
    }
    if (scales[2] == 1.0f) {
        selectedPNG = std::move(sdPng);
        selectedPlist = std::move(sdPlist);
    }

    if (auto icon = more_icons::getIcon(name, type)) {
        more_icons::updateIcon(icon);
    }
    else {
        icon = more_icons::addIcon(name, name, type, std::move(selectedPNG), std::move(selectedPlist), Get::Director()->getLoadedTextureQuality());
        if (Icons::preloadIcons) Icons::createAndAddFrames(icon);
    }

    m_parentPopup1->close();
    m_parentPopup2->close();
    Popup::onClose(nullptr);

    Notify::success("{} saved!", name);
    MoreIcons::updateGarage();
}

void SaveIconPopup::onClose(CCObject* sender) {
    if (m_nameInput->getString().empty()) return Popup::onClose(sender);

    auto type = m_iconType;
    createQuickPopup(
        fmt::format("Exit {} Saver", Constants::getSingularUppercase(type)).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} saver</c>?", Constants::getSingularLowercase(type)),
        "No",
        "Yes",
        [this](auto, bool btn2) {
            if (btn2) Popup::onClose(m_closeBtn);
        }
    );
}
