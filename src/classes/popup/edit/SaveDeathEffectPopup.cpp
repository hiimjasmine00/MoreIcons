#include "SaveDeathEffectPopup.hpp"
#include "ImageRenderer.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Defaults.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Icons.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/file.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

SaveDeathEffectPopup* SaveDeathEffectPopup::create(
    BasePopup* popup1, BasePopup* popup2, const std::vector<FrameDefinition>& definitions, const std::vector<Ref<CCSpriteFrame>>& frames
) {
    auto ret = new SaveDeathEffectPopup();
    if (ret->init(popup1, popup2, definitions, frames)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SaveDeathEffectPopup::init(
    BasePopup* popup1, BasePopup* popup2, const std::vector<FrameDefinition>& definitions, const std::vector<Ref<CCSpriteFrame>>& frames
) {
    if (!BasePopup::init(350.0f, 130.0f, "geode.loader/GE_square03.png", CircleBaseColor::DarkPurple)) return false;

    setID("SaveDeathEffectPopup");
    setTitle("Save Death Effect");
    m_title->setID("save-icon-title");

    m_parentPopup1 = popup1;
    m_parentPopup2 = popup2;
    m_definitions = &definitions;
    m_frames = &frames;

    auto nameInput = TextInput::create(300.0f, "Death Effect Name");
    nameInput->setPosition({ 175.0f, 70.0f });
    nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    nameInput->setMaxCharCount(100);
    nameInput->setID("name-input");
    m_mainLayer->addChild(nameInput);

    m_nameInput = nameInput->getInputNode()->m_textField;

    auto saveButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f), this, menu_selector(SaveDeathEffectPopup::onSave)
    );
    saveButton->setPosition({ 175.0f, 30.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}

void SaveDeathEffectPopup::onSave(CCObject* sender) {
    auto effectName = MoreIcons::getText(m_nameInput);
    if (effectName.empty()) return Notify::info("Please enter a name.");

    m_pendingPath = MoreIcons::getIconStem(effectName, IconType::DeathEffect);
    if (Filesystem::doesExist(m_pendingPath) && (
        Filesystem::doesExist(m_pendingPath / L("effect-uhd.png")) ||
        Filesystem::doesExist(m_pendingPath / L("effect-hd.png")) ||
        Filesystem::doesExist(m_pendingPath / L("effect.png")) ||
        Filesystem::doesExist(m_pendingPath / L("effect-uhd.plist")) ||
        Filesystem::doesExist(m_pendingPath / L("effect-hd.plist")) ||
        Filesystem::doesExist(m_pendingPath / L("effect.plist"))
    )) {
        auto alert = FLAlertLayer::create(
            this,
            "Existing Icon",
            fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", effectName),
            "No",
            "Yes",
            350.0f
        );
        alert->setTag(0);
        alert->show();
    }
    else saveIcon();
}

void SaveDeathEffectPopup::saveIcon() {
    if (auto res = file::createDirectoryAll(m_pendingPath); res.isErr()) {
        return Notify::error(res.unwrapErr());
    }

    auto name = MoreIcons::getText(m_nameInput);

    std::array<texpack::Packer, 3> packers = {};
    auto scaleFactor = Get::director->getContentScaleFactor();
    std::array scales = { 4.0f / scaleFactor, 2.0f / scaleFactor, 1.0f / scaleFactor };
    for (size_t i = 0; i < m_frames->size(); i++) {
        auto& frameRef = (*m_frames)[i];
        auto& definition = (*m_definitions)[i];

        auto joinedName = fmt::format("effect_{:03}.png", i + 1);
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

    auto uhdPngPath = m_pendingPath / L("effect-uhd.png");
    auto hdPngPath = m_pendingPath / L("effect-hd.png");
    auto sdPngPath = m_pendingPath / L("effect.png");
    auto uhdPlistPath = m_pendingPath / L("effect-uhd.plist");
    auto hdPlistPath = m_pendingPath / L("effect-hd.plist");
    auto sdPlistPath = m_pendingPath / L("effect.plist");

    if (auto res = ImageRenderer::save(packers[0], uhdPngPath, uhdPlistPath, "effect-uhd.png"); res.isErr()) {
        return Notify::error("Failed to save UHD icon: {}", res.unwrapErr());
    }

    if (auto res = ImageRenderer::save(packers[1], hdPngPath, hdPlistPath, "effect-hd.png"); res.isErr()) {
        return Notify::error("Failed to save HD icon: {}", res.unwrapErr());
    }

    if (auto res = ImageRenderer::save(packers[2], sdPngPath, sdPlistPath, "effect.png"); res.isErr()) {
        return Notify::error("Failed to save SD icon: {}", res.unwrapErr());
    }

    if (auto icon = more_icons::getIcon(name, IconType::DeathEffect)) {
        more_icons::updateIcon(icon);
    }
    else {
        std::filesystem::path pngPath;
        std::filesystem::path plistPath;
        if (scaleFactor >= 4.0f) {
            pngPath = std::move(uhdPngPath);
            plistPath = std::move(uhdPlistPath);
        }
        else if (scaleFactor >= 2.0f) {
            pngPath = std::move(hdPngPath);
            plistPath = std::move(hdPlistPath);
        }
        else {
            pngPath = std::move(sdPngPath);
            plistPath = std::move(sdPlistPath);
        }

        auto jsonPath = m_pendingPath / L("settings.json");
        (void)file::writeString(jsonPath, "{}");
        icon = more_icons::addDeathEffect(
            name, name, std::move(pngPath), std::move(plistPath), std::move(jsonPath), {},
            Get::director->getLoadedTextureQuality(), {}, "More Icons", 0, Defaults::getDeathEffectInfo(0)
        );
        if (Icons::preloadIcons) Icons::createAndAddFrames(icon);
    }

    auto notif = fmt::format("{} saved!", name);

    close();
    m_parentPopup2->close();
    m_parentPopup1->close();

    Notify::success(notif);
    MoreIcons::updateGarage();
}

void SaveDeathEffectPopup::onClose(CCObject* sender) {
    if (MoreIcons::getText(m_nameInput).empty()) return close();

    auto alert = FLAlertLayer::create(
        this,
        "Exit Death Effect Saver",
        "Are you sure you want to <cy>exit</c> the <cg>death effect saver</c>?",
        "No",
        "Yes",
        350.0f
    );
    alert->setTag(1);
    alert->show();
}

void SaveDeathEffectPopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (!btn2) return;
    switch (layer->getTag()) {
        case 0: saveIcon(); break;
        case 1: close(); break;
    }
}
