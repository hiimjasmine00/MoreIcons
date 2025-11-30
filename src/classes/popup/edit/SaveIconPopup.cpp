#include "SaveIconPopup.hpp"
#include "EditIconPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Notification.hpp>
#include <texpack.hpp>

using namespace geode::prelude;

template <typename... T>
void notify(NotificationIcon icon, fmt::format_string<T...> message, T&&... args) {
    Notification::create(fmt::format(message, std::forward<T>(args)...), icon)->show();
}

SaveIconPopup* SaveIconPopup::create(EditIconPopup* popup, IconType type, CCDictionary* definitions, CCDictionary* frames) {
    auto ret = new SaveIconPopup();
    if (ret->initAnchored(350.0f, 130.0f, popup, type, definitions, frames, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SaveIconPopup::setup(EditIconPopup* popup, IconType type, CCDictionary* definitions, CCDictionary* frames) {
    auto miType = (int)type;
    setID("SaveIconPopup");
    setTitle(fmt::format("Save {}", MoreIconsAPI::uppercase[miType]));
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
    auto saveButton = CCMenuItemExt::createSpriteExtra(saveSprite, [this, miType](auto) {
        auto iconName = m_nameInput->getString();
        if (iconName.empty()) return notify(NotificationIcon::Info, "Please enter a name.");

        auto parent = Mod::get()->getConfigDir() / MoreIcons::folders[miType];
        auto factor = MoreIconsAPI::getDirector()->getContentScaleFactor();
        auto suffix = factor >= 4.0f ? "-uhd" : factor >= 2.0f ? "-hd" : "";
        auto png = parent / fmt::format("{}{}.png", iconName, suffix);
        auto plist = parent / fmt::format("{}{}.plist", iconName, suffix);
        if (MoreIcons::doesExist(png) || MoreIcons::doesExist(plist)) {
            createQuickPopup(
                "Existing Icon",
                fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", iconName),
                "No",
                "Yes",
                [this, png = std::move(png), plist = std::move(plist)](auto, bool btn2) {
                    if (btn2) saveIcon(png, plist);
                }
            );
        }
        else saveIcon(png, plist);
    });
    saveButton->setPosition({ 175.0f, 30.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}

texpack::Image getImage(CCNode* node) {
    auto director = MoreIconsAPI::getDirector();
    auto size = node->getContentSize() * director->getContentScaleFactor();
    auto floatWidth = size.width;
    auto floatHeight = size.height;
    uint32_t width = floatWidth;
    uint32_t height = floatHeight;

    auto texture = 0u;
    glPixelStorei(GL_PACK_ALIGNMENT, 8);
    glGenTextures(1, &texture);
    ccGLBindTexture2D(texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    auto oldFBO = 0;
    auto fbo = 0u;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    kmGLMatrixMode(KM_GL_PROJECTION);
	kmGLPushMatrix();
	kmGLMatrixMode(KM_GL_MODELVIEW);
    kmGLPushMatrix();

    glViewport(0, 0, width, height);

    auto winSize = director->getWinSizeInPixels();

    kmMat4 orthoMatrix;
    kmMat4OrthographicProjection(&orthoMatrix, -floatWidth / winSize.width, floatWidth / winSize.width,
        -floatHeight / winSize.height, floatHeight / winSize.height, -1.0f, 1.0f);
    kmGLMultMatrix(&orthoMatrix);

    node->visit();

    std::vector<uint8_t> data(width * height * 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
    director->setViewport();
    kmGLMatrixMode(KM_GL_PROJECTION);
    kmGLPopMatrix();
    kmGLMatrixMode(KM_GL_MODELVIEW);
    kmGLPopMatrix();
    ccGLDeleteTexture(texture);
    glDeleteFramebuffers(1, &fbo);

    for (int y = 0; y < height / 2; y++) {
        std::swap_ranges(data.begin() + y * width * 4, data.begin() + (y + 1) * width * 4, data.end() - (y + 1) * width * 4);
    }

    return { data, width, height };
}

bool SaveIconPopup::checkFrame(std::string_view suffix) {
    auto frame = m_frames->objectForKey(gd::string(suffix.data(), suffix.size()));
    if (!frame) notify(NotificationIcon::Info, "Missing {}{}.", m_nameInput->getString(), suffix);
    return frame != nullptr;
}

void SaveIconPopup::saveIcon(const std::filesystem::path& png, const std::filesystem::path& plist) {
    auto type = m_iconType;
    if (type == IconType::Robot || type == IconType::Spider) {
        if (!checkFrame("_01_001.png") || !checkFrame("_01_2_001.png") || !checkFrame("_01_glow_001.png")) return;
        if (!checkFrame("_02_001.png") || !checkFrame("_02_2_001.png") || !checkFrame("_02_glow_001.png")) return;
        if (!checkFrame("_03_001.png") || !checkFrame("_03_2_001.png") || !checkFrame("_03_glow_001.png")) return;
        if (!checkFrame("_04_001.png") || !checkFrame("_04_2_001.png") || !checkFrame("_04_glow_001.png")) return;
    }
    else {
        if (!checkFrame("_001.png")) return;
        if (!checkFrame("_2_001.png")) return;
        if (type == IconType::Ufo && !checkFrame("_3_001.png")) return;
        if (!checkFrame("_glow_001.png")) return;
    }

    auto name = m_nameInput->getString();

    texpack::Packer packer;
    for (auto [frameName, frame] : CCDictionaryExt<std::string_view, CCSpriteFrame*>(m_frames)) {
        auto suffix = frameName.substr(0, frameName.size() - 4);
        auto offsetX = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("offsetX{}", suffix)))->getValue();
        auto offsetY = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("offsetY{}", suffix)))->getValue();
        auto rotationX = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("rotationX{}", suffix)))->getValue();
        auto rotationY = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("rotationY{}", suffix)))->getValue();
        auto scaleX = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("scaleX{}", suffix)))->getValue();
        auto scaleY = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("scaleY{}", suffix)))->getValue();

        auto node = CCNode::create();
        auto sprite = CCSprite::createWithSpriteFrame(frame);
        sprite->setPosition({ offsetX, offsetY });
        sprite->setScaleX(scaleX);
        sprite->setScaleY(scaleY);
        sprite->setRotationX(rotationX);
        sprite->setRotationY(rotationY);
        node->addChild(sprite);
        node->setAnchorPoint({ 0.0f, 0.0f });
        auto boundingSize = sprite->boundingBox().size;
        node->setContentSize(boundingSize + CCSize { std::abs(offsetX * 2.0f), std::abs(offsetY * 2.0f) });
        sprite->setPosition(node->getContentSize() * 0.5f + CCPoint { offsetX, offsetY });
        packer.frame(fmt::format("{}{}", name, frameName), getImage(node));
        node->release();
        sprite->release();
    }

    if (auto res = packer.pack(); res.isErr()) {
        return notify(NotificationIcon::Error, "Failed to pack frames: {}", res.unwrapErr());
    }
    if (auto res = packer.png(png); res.isErr()) {
        return notify(NotificationIcon::Error, "Failed to save image: {}", res.unwrapErr());
    }
    if (auto res = packer.plist(plist, fmt::format("icons/{}", string::pathToString(png.filename())), "    "); res.isErr()) {
        return notify(NotificationIcon::Error, "Failed to save plist: {}", res.unwrapErr());
    }

    addOrUpdateIcon(name, png, plist);

    m_parentPopup->close();
    Popup::onClose(nullptr);
}

void SaveIconPopup::addOrUpdateIcon(const std::string& name, const std::filesystem::path& png, const std::filesystem::path& plist) {
    auto type = m_iconType;
    if (auto icon = MoreIconsAPI::getIcon(name, type)) MoreIconsAPI::updateIcon(icon);
    else {
        icon = MoreIconsAPI::addIcon(name, name, type,
            string::pathToString(png), string::pathToString(plist), "", "More Icons", 0, {}, false, false);
        if (MoreIconsAPI::preloadIcons) {
            if (auto res = MoreIconsAPI::createFrames(icon->textures[0], icon->sheetName, icon->name, icon->type)) {
                MoreIconsAPI::addFrames(res.unwrap(), icon->frameNames);
            }
            else {
                log::error("{}: {}", icon->name, res.unwrapErr());
            }
        }
    }

    m_parentPopup->close();
    Popup::onClose(nullptr);

    notify(NotificationIcon::Success, "{} saved!", name);
    MoreIcons::updateGarage();
}

void SaveIconPopup::onClose(CCObject* sender) {
    if (m_nameInput->getString().empty()) return Popup::onClose(sender);

    auto type = MoreIconsAPI::convertType(m_iconType);
    createQuickPopup(
        fmt::format("Exit {} Saver", MoreIconsAPI::uppercase[type]).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} saver</c>?", MoreIconsAPI::lowercase[type]),
        "No",
        "Yes",
        [this](auto, bool btn2) {
            if (btn2) Popup::onClose(m_closeBtn);
        }
    );
}
