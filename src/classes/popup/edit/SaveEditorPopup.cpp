#include "SaveEditorPopup.hpp"
#include "ImageRenderer.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/utils/file.hpp>

using namespace geode::prelude;

SaveEditorPopup* SaveEditorPopup::create(
    IconType type, const IconEditorState& state, const StringMap<Ref<CCSpriteFrame>>& frames, Function<void()> callback
) {
    auto ret = new SaveEditorPopup();
    if (ret->init(type, state, frames, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SaveEditorPopup::init(
    IconType type, const IconEditorState& state, const StringMap<Ref<CCSpriteFrame>>& frames, Function<void()> callback
) {
    if (!BasePopup::init(350.0f, 130.0f, "geode.loader/GE_square03.png")) return false;

    setID("SaveEditorPopup");
    setTitle(fmt::format("Save {} Editor", Constants::getSingularUppercase(type)));
    m_title->setID("save-editor-title");

    m_callback = std::move(callback);
    m_iconType = type;
    m_state = &state;
    m_frames = &frames;

    m_nameInput = TextInput::create(300.0f, "State Name");
    m_nameInput->setPosition({ 175.0f, 70.0f });
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_nameInput->setMaxCharCount(100);
    m_nameInput->setID("name-input");
    m_mainLayer->addChild(m_nameInput);

    auto saveButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f), this, menu_selector(SaveEditorPopup::onSave)
    );
    saveButton->setPosition({ 175.0f, 30.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}

void SaveEditorPopup::onSave(CCObject* sender) {
    auto stateName = MoreIcons::getText(m_nameInput);
    if (stateName.empty()) return Notify::info("Please enter a name.");

    m_pendingPath = MoreIcons::getEditorDir(m_iconType) / Filesystem::strWide(stateName);
    if (Filesystem::doesExist(m_pendingPath)) {
        auto alert = FLAlertLayer::create(
            this,
            "Existing State",
            fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", stateName),
            "No",
            "Yes",
            350.0f
        );
        alert->setTag(0);
        alert->show();
    }
    else saveEditor();
}

void SaveEditorPopup::saveEditor() {
    if (!Filesystem::doesExist(m_pendingPath)) {
        if (auto res = file::createDirectoryAll(m_pendingPath); res.isErr()) {
            return Notify::error(res.unwrapErr());
        }
    }

    if (auto res = file::writeToJson(m_pendingPath / L("state.json"), *m_state); res.isErr()) {
        return Notify::error("Failed to save state: {}", res.unwrapErr());
    }

    texpack::Packer packer;
    for (auto& [frameName, frame] : *m_frames) {
        auto sprite = CCSprite::createWithSpriteFrame(frame);
        sprite->setAnchorPoint({ 0.0f, 0.0f });
        sprite->setBlendFunc({ GL_ONE, GL_ZERO });
        packer.frame(fmt::format("icon{}.png", frameName), ImageRenderer::getImage(sprite));
        sprite->release();
    }

    if (auto res = ImageRenderer::save(packer, m_pendingPath / L("icon.png"), m_pendingPath / L("icon.plist"), "icon.png"); res.isErr()) {
        return Notify::error(res.unwrapErr());
    }

    auto notif = fmt::format("{} saved!", MoreIcons::getText(m_nameInput));

    m_callback();
    close();

    Notify::success(notif);
}

void SaveEditorPopup::onClose(CCObject* sender) {
    if (MoreIcons::getText(m_nameInput).empty()) return close();

    auto type = m_iconType;
    auto alert = FLAlertLayer::create(
        this,
        fmt::format("Exit {} Editor Saver", Constants::getSingularUppercase(type)).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} editor saver</c>?", Constants::getSingularLowercase(type)),
        "No",
        "Yes",
        350.0f
    );
    alert->setTag(1);
    alert->show();
}

void SaveEditorPopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (!btn2) return;
    switch (layer->getTag()) {
        case 0: saveEditor(); break;
        case 1: close(); break;
    }
}
