#include "SaveIconPopup.hpp"
#include "EditIconPopup.hpp"
#include "ImageRenderer.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Load.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

SaveIconPopup* SaveIconPopup::create(EditIconPopup* popup, IconType type, const matjson::Value& definitions, CCDictionary* frames) {
    auto ret = new SaveIconPopup();
    if (ret->initAnchored(350.0f, 130.0f, popup, type, definitions, frames, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SaveIconPopup::setup(EditIconPopup* popup, IconType type, const matjson::Value& definitions, CCDictionary* frames) {
    setID("SaveIconPopup");
    setTitle(fmt::format("Save {}", MoreIcons::uppercase[(int)type]));
    m_title->setID("save-icon-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_parentPopup = popup;
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
        if (iconName.empty()) return MoreIcons::notifyInfo("Please enter a name.");

        auto stem = MoreIcons::getIconStem(iconName, m_iconType);
        auto& stemStr = stem.native();
        if (
            MoreIcons::doesExist(stemStr + MI_PATH(".png")) ||
            MoreIcons::doesExist(stemStr + MI_PATH("-hd.png")) ||
            MoreIcons::doesExist(stemStr + MI_PATH("-uhd.png")) ||
            MoreIcons::doesExist(stemStr + MI_PATH(".plist")) ||
            MoreIcons::doesExist(stemStr + MI_PATH("-hd.plist")) ||
            MoreIcons::doesExist(stemStr + MI_PATH("-uhd.plist"))
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

bool SaveIconPopup::checkFrame(const std::string& suffix) {
    auto frame = m_frames->objectForKey(suffix);
    if (!frame) MoreIcons::notifyInfo("Missing {}{}.", m_nameInput->getString(), suffix);
    return frame != nullptr;
}

void SaveIconPopup::saveIcon(const std::filesystem::path& stem) {
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

    texpack::Packer packers[3];
    auto scaleFactor = Get::Director()->getContentScaleFactor();
    float scales[3] = { 4.0f / scaleFactor, 2.0f / scaleFactor, 1.0f / scaleFactor };
    constexpr std::array suffixes = {
        std::make_tuple(MI_PATH("-uhd"), "-uhd", "UHD"),
        std::make_tuple(MI_PATH("-hd"), "-hd", "HD"),
        std::make_tuple(MI_PATH(""), "", "SD")
    };
    for (auto [frameName, frame] : CCDictionaryExt<std::string_view, CCSpriteFrame*>(m_frames)) {
        auto& definition = m_definitions[frameName];
        auto offsetX = definition.get<float>("offset-x").unwrapOr(0.0f);
        auto offsetY = definition.get<float>("offset-y").unwrapOr(0.0f);
        auto rotationX = definition.get<float>("rotation-x").unwrapOr(0.0f);
        auto rotationY = definition.get<float>("rotation-y").unwrapOr(0.0f);
        auto scaleX = definition.get<float>("scale-x").unwrapOr(1.0f);
        auto scaleY = definition.get<float>("scale-y").unwrapOr(1.0f);
        auto joinedName = fmt::format("{}{}.png", name, frameName);
        for (int i = 0; i < 3; i++) {
            auto node = CCNode::create();
            node->setScale(scales[i]);
            node->setAnchorPoint({ 0.0f, 0.0f });
            auto sprite = CCSprite::createWithSpriteFrame(frame);
            sprite->setPosition({ offsetX, offsetY });
            sprite->setScaleX(scaleX);
            sprite->setScaleY(scaleY);
            sprite->setRotationX(rotationX);
            sprite->setRotationY(rotationY);
            node->addChild(sprite);
            auto boundingSize = sprite->boundingBox().size;
            node->setContentSize(boundingSize + CCSize { std::abs(offsetX * 2.0f), std::abs(offsetY * 2.0f) });
            sprite->setPosition(node->getContentSize() / 2.0f + sprite->getPosition());
            sprite->setBlendFunc({ GL_ONE, GL_ZERO });
            packers[i].frame(joinedName, ImageRenderer::getImage(node));
            node->release();
            sprite->release();
        }
    }

    std::filesystem::path selectedPNG;
    std::filesystem::path selectedPlist;
    for (int i = 0; i < 3; i++) {
        auto& packer = packers[i];
        auto [wsuffix, suffix, displayName] = suffixes[i];
        auto start = stem.native() + wsuffix;
        std::filesystem::path png = start + MI_PATH(".png");
        std::filesystem::path plist = start + MI_PATH(".plist");
        if (scales[i] == 1.0f) {
            selectedPNG = png;
            selectedPlist = plist;
        }
        if (auto res = packer.pack(); res.isErr()) {
            return MoreIcons::notifyFailure("Failed to pack {} frames: {}", displayName, res.unwrapErr());
        }
        if (auto res = packer.png(png); res.isErr()) {
            return MoreIcons::notifyFailure("Failed to save {} image: {}", displayName, res.unwrapErr());
        }
        if (auto res = packer.plist(plist, fmt::format("icons/{}{}.png", name, suffix), "    "); res.isErr()) {
            return MoreIcons::notifyFailure("Failed to save {} plist: {}", displayName, res.unwrapErr());
        }
    }

    addOrUpdateIcon(name, selectedPNG, selectedPlist);

    m_parentPopup->close();
    Popup::onClose(nullptr);
}

void SaveIconPopup::addOrUpdateIcon(const std::string& name, const std::filesystem::path& png, const std::filesystem::path& plist) {
    auto type = m_iconType;
    if (auto icon = more_icons::getIcon(name, type)) more_icons::updateIcon(icon);
    else {
        icon = more_icons::addIcon(name, name, type, png, plist);
        if (MoreIcons::preloadIcons) MoreIcons::createAndAddFrames(icon);
    }

    m_parentPopup->close();
    Popup::onClose(nullptr);

    MoreIcons::notifySuccess("{} saved!", name);
    MoreIcons::updateGarage();
}

void SaveIconPopup::onClose(CCObject* sender) {
    if (m_nameInput->getString().empty()) return Popup::onClose(sender);

    auto type = (int)m_iconType;
    createQuickPopup(
        fmt::format("Exit {} Saver", MoreIcons::uppercase[type]).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} saver</c>?", MoreIcons::lowercase[type]),
        "No",
        "Yes",
        [this](auto, bool btn2) {
            if (btn2) Popup::onClose(m_closeBtn);
        }
    );
}
