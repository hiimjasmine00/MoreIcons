#include "SaveEffectEditorPopup.hpp"
#include "../ImageRenderer.hpp"
#include "../../../../MoreIcons.hpp"
#include "../../../../utils/Constants.hpp"
#include "../../../../utils/Filesystem.hpp"
#include "../../../../utils/Get.hpp"
#include "../../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/file.hpp>
#include <matjson/std.hpp>

using namespace geode::prelude;

SaveEffectEditorPopup* SaveEffectEditorPopup::create(
    const std::vector<FrameDefinition>& definitions, const std::vector<Ref<CCSpriteFrame>>& frames, Function<void()> callback
) {
    auto ret = new SaveEffectEditorPopup();
    if (ret->init(definitions, frames, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SaveEffectEditorPopup::init(
    const std::vector<FrameDefinition>& definitions, const std::vector<Ref<CCSpriteFrame>>& frames, Function<void()> callback
) {
    if (!BasePopup::init(350.0f, 130.0f, "geode.loader/GE_square03.png", CircleBaseColor::DarkPurple)) return false;

    setID("SaveEffectEditorPopup");
    setTitle("Save Death Effect Editor");
    m_title->setID("save-effect-editor-title");

    m_callback = std::move(callback);
    m_definitions = &definitions;
    m_frames = &frames;

    auto nameInput = TextInput::create(300.0f, "State Name");
    nameInput->setPosition({ 175.0f, 70.0f });
    nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    nameInput->setMaxCharCount(100);
    nameInput->setID("name-input");
    m_mainLayer->addChild(nameInput);

    m_nameInput = nameInput->getInputNode()->m_textField;

    auto saveButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f), this, menu_selector(SaveEffectEditorPopup::onSave)
    );
    saveButton->setPosition({ 175.0f, 30.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}

void SaveEffectEditorPopup::onSave(CCObject* sender) {
    auto stateName = MoreIcons::getText(m_nameInput);
    if (stateName.empty()) return Notify::info("Please enter a name.");

    m_pendingPath = MoreIcons::getEditorDir(IconType::DeathEffect) / Filesystem::strWide(stateName);
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

void SaveEffectEditorPopup::saveEditor() {
    if (!Filesystem::doesExist(m_pendingPath)) {
        if (auto res = file::createDirectoryAll(m_pendingPath); res.isErr()) {
            return Notify::error(res.unwrapErr());
        }
    }

    if (auto res = file::writeToJson(m_pendingPath / L("state.json"), matjson::makeObject({
        { std::string("definitions", 11), matjson::Value(*m_definitions) }
    })); res.isErr()) {
        return Notify::error("Failed to save state: {}", res.unwrapErr());
    }

    texpack::Packer packer;
    for (size_t i = 0; i < m_frames->size(); ++i) {
        auto sprite = CCSprite::createWithSpriteFrame((*m_frames)[i]);
        sprite->setAnchorPoint({ 0.0f, 0.0f });
        sprite->setBlendFunc({ GL_ONE, GL_ZERO });
        packer.frame(fmt::format("effect_{:03}.png", i + 1), ImageRenderer::getImage(sprite));
        sprite->release();
    }

    if (auto res = ImageRenderer::save(packer, m_pendingPath / L("effect.png"), m_pendingPath / L("effect.plist"), "effect.png"); res.isErr()) {
        return Notify::error(res.unwrapErr());
    }

    auto notif = fmt::format("{} saved!", MoreIcons::getText(m_nameInput));

    m_callback();
    close();

    Notify::success(notif);
}

void SaveEffectEditorPopup::onClose(CCObject* sender) {
    if (MoreIcons::getText(m_nameInput).empty()) return close();

    auto alert = FLAlertLayer::create(
        this,
        "Exit Death Effect Editor Saver",
        "Are you sure you want to <cy>exit</c> the <cg>death effect editor saver</c>?",
        "No",
        "Yes",
        400.0f
    );
    alert->setTag(1);
    alert->show();
}

void SaveEffectEditorPopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (!btn2) return;
    switch (layer->getTag()) {
        case 0: saveEditor(); break;
        case 1: close(); break;
    }
}
