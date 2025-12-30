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
    IconType type, const IconEditorState& state,
    const std::unordered_map<std::string, Ref<CCSpriteFrame>>& frames, std23::move_only_function<void()> callback
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
    IconType type, const IconEditorState& state,
    const std::unordered_map<std::string, Ref<CCSpriteFrame>>& frames, std23::move_only_function<void()> callback
) {
    if (!BasePopup::init(350.0f, 130.0f, "geode.loader/GE_square03.png")) return false;

    setID("SaveEditorPopup");
    setTitle(fmt::format("Save {} Editor", Constants::getSingularUppercase(type)));
    m_title->setID("save-editor-title");

    m_callback = std::move(callback);
    m_iconType = type;
    m_state = state;
    m_frames = &frames;

    m_nameInput = TextInput::create(300.0f, "State Name");
    m_nameInput->setPosition({ 175.0f, 70.0f });
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_nameInput->setMaxCharCount(100);
    m_nameInput->setID("name-input");
    m_mainLayer->addChild(m_nameInput);

    auto saveSprite = ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto saveButton = CCMenuItemExt::createSpriteExtra(saveSprite, [this](auto) {
        auto stateName = m_nameInput->getString();
        if (stateName.empty()) return Notify::info("Please enter a name.");

        auto directory = MoreIcons::getEditorDir(m_iconType) / Filesystem::strWide(stateName);
        if (Filesystem::doesExist(directory)) {
            createQuickPopup(
                "Existing State",
                fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", stateName),
                "No",
                "Yes",
                [this, directory = std::move(directory)](auto, bool btn2) {
                    if (btn2) saveEditor(directory);
                }
            );
        }
        else saveEditor(directory);
    });
    saveButton->setPosition({ 175.0f, 30.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}

void SaveEditorPopup::saveEditor(const std::filesystem::path& directory) {
    if (!Filesystem::doesExist(directory)) {
        if (auto res = file::createDirectoryAll(directory); res.isErr()) {
            return Notify::error(res.unwrapErr());
        }
    }

    if (auto res = file::writeToJson(directory / L("state.json"), m_state); res.isErr()) {
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

    if (auto res = ImageRenderer::save(packer, directory / L("icon.png"), directory / L("icon.plist"), "icon.png"); res.isErr()) {
        return Notify::error(res.unwrapErr());
    }

    m_callback();
    Popup::onClose(nullptr);

    Notify::success("{} saved!", m_nameInput->getString());
}

void SaveEditorPopup::onClose(CCObject* sender) {
    if (m_nameInput->getString().empty()) return Popup::onClose(sender);

    auto type = m_iconType;
    createQuickPopup(
        fmt::format("Exit {} Editor Saver", Constants::getSingularUppercase(type)).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} editor saver</c>?", Constants::getSingularLowercase(type)),
        "No",
        "Yes",
        [this](auto, bool btn2) {
            if (btn2) Popup::onClose(m_closeBtn);
        }
    );
}
