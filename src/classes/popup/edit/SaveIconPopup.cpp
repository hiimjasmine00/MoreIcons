#include "SaveIconPopup.hpp"
#include "ImageRenderer.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Icons.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/TextInput.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

SaveIconPopup* SaveIconPopup::create(
    IconType type, bool editor, Function<bool(ZStringView)> checkCallback,
    Function<Result<>(ZStringView)> saveCallback, Function<void()> closeCallback
) {
    auto ret = new SaveIconPopup();
    if (ret->init(type, editor, std::move(checkCallback), std::move(saveCallback), std::move(closeCallback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SaveIconPopup::init(
    IconType type, bool editor, Function<bool(ZStringView)> checkCallback,
    Function<Result<>(ZStringView)> saveCallback, Function<void()> closeCallback
) {
    if (!BasePopup::init(350.0f, 130.0f, "geode.loader/GE_square03.png", CircleBaseColor::DarkPurple)) return false;

    setID("SaveIconPopup");
    if (editor) setTitle(fmt::format("Save {} Editor", Constants::getSingularUppercase(type)));
    else setTitle(fmt::format("Save {}", Constants::getSingularUppercase(type)));
    m_title->setID("save-icon-title");

    m_iconType = type;
    m_editor = editor;
    m_checkCallback = std::move(checkCallback);
    m_saveCallback = std::move(saveCallback);
    m_closeCallback = std::move(closeCallback);

    auto nameInput = TextInput::create(300.0f, "Icon Name");
    nameInput->setPosition({ 175.0f, 70.0f });
    nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    nameInput->setMaxCharCount(100);
    nameInput->setID("name-input");
    m_mainLayer->addChild(nameInput);

    m_nameInput = nameInput->getInputNode()->m_textField;

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

    if (m_checkCallback(iconName)) {
        auto alert = FLAlertLayer::create(
            this,
            m_editor ? "Existing State" : "Existing Icon",
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

    if (auto res = m_saveCallback(name); res.isErr()) {
        return Notify::error(res.unwrapErr());
    }

    MoreIcons::updateGarageAndNotify(fmt::format("{} saved!", name));
}

void SaveIconPopup::onClose(CCObject* sender) {
    if (MoreIcons::getText(m_nameInput).empty()) {
        m_closeCallback();
        return close();
    }

    auto type = m_iconType;
    if (m_editor) {
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
    else {
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
}

void SaveIconPopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (!btn2) return;
    switch (layer->getTag()) {
        case 0:
            saveIcon();
            break;
        case 1:
            m_closeCallback();
            close();
            break;
    }
}
