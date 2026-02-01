#include "EditTrailPopup.hpp"
#include "IconPresetPopup.hpp"
#include "ImageRenderer.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Defaults.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Icons.hpp"
#include "../../../utils/Load.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/utils/file.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

EditTrailPopup* EditTrailPopup::create(BasePopup* popup) {
    auto ret = new EditTrailPopup();
    if (ret->init(popup)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool EditTrailPopup::init(BasePopup* popup) {
    if (!BasePopup::init(350.0f, 180.0f, "geode.loader/GE_square03.png")) return false;

    setID("EditTrailPopup");
    setTitle("Trail Editor");
    m_title->setID("edit-trail-title");

    m_parentPopup = popup;

    m_streak = CCSprite::create("streak_01_001.png");
    m_streak->setPosition({ 175.0f, 120.0f });
    m_streak->setRotation(-90.0f);
    auto& size = m_streak->getContentSize();
    m_streak->setScaleX(14.0f / size.width);
    m_streak->setScaleY(320.0f / size.height);
    m_streak->setID("streak-preview");
    m_mainLayer->addChild(m_streak);

    m_nameInput = TextInput::create(300.0f, "Name");
    m_nameInput->setPosition({ 175.0f, 75.0f });
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_nameInput->setMaxCharCount(100);
    m_nameInput->setID("text-input");
    m_mainLayer->addChild(m_nameInput);

    auto bottomMenu = CCMenu::create();
    bottomMenu->setPosition({ 175.0f, 30.0f });
    bottomMenu->setContentSize({ 350.0f, 30.0f });
    bottomMenu->ignoreAnchorPointForPosition(false);
    bottomMenu->setID("bottom-menu");
    m_mainLayer->addChild(bottomMenu);

    auto pngButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("PNG", "goldFont.fnt", "GJ_button_05.png"), this, menu_selector(EditTrailPopup::onPNG)
    );
    pngButton->setID("png-button");
    bottomMenu->addChild(pngButton);

    auto presetButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png"), this, menu_selector(EditTrailPopup::onPreset)
    );
    presetButton->setID("preset-button");
    bottomMenu->addChild(presetButton);

    auto saveButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png"), this, menu_selector(EditTrailPopup::onSave)
    );
    saveButton->setID("save-button");
    bottomMenu->addChild(saveButton);

    bottomMenu->setLayout(RowLayout::create()->setGap(25.0f));

    handleTouchPriority(this);

    return true;
}

void EditTrailPopup::onPNG(CCObject* sender) {
    m_listener.spawn(file::pick(file::PickMode::OpenFile, {
        .filters = {{
            .description = "PNG files",
            .files = { "*.png" }
        }}
    }), [this](Result<std::optional<std::filesystem::path>> res) {
        if (res.isErr()) return Notify::error("Failed to import PNG file: {}", res.unwrapErr());

        auto path = std::move(res).unwrap();
        if (!path.has_value()) return;

        updateWithPath(path.value());
    });
}

void EditTrailPopup::onPreset(CCObject* sender) {
    IconPresetPopup::create(IconType::Special, {}, [this](int id, IconInfo* info) {
        updateWithPath(MoreIcons::getIconPath(info, id, IconType::Special));
    })->show();
}

void EditTrailPopup::onSave(CCObject* sender) {
    auto iconName = MoreIcons::getText(m_nameInput);
    if (iconName.empty()) return Notify::info("Please enter a name.");

    m_pendingPath = MoreIcons::getIconStem(fmt::format("{}.png", iconName), IconType::Special);
    if (Filesystem::doesExist(m_pendingPath)) {
        auto alert = FLAlertLayer::create(
            this,
            "Existing Trail",
            fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", iconName),
            "No",
            "Yes",
            350.0f
        );
        alert->setTag(0);
        alert->show();
    }
    else saveTrail();
}

void EditTrailPopup::updateWithPath(const std::filesystem::path& path) {
    if (auto textureRes = Load::createTexture(path); textureRes.isOk()) {
        m_streak->setTexture(textureRes.unwrap());
        m_hasChanged = true;
    }
    else if (textureRes.isErr()) Notify::error(textureRes.unwrapErr());
}

void EditTrailPopup::saveTrail() {
    auto sprite = CCSprite::createWithTexture(m_streak->getTexture());
    sprite->setAnchorPoint({ 0.0f, 0.0f });
    sprite->setBlendFunc({ GL_ONE, GL_ZERO });
    auto image = ImageRenderer::getImage(sprite);
    sprite->release();
    auto imageRes = texpack::toPNG(image);
    if (imageRes.isErr()) {
        return Notify::error("Failed to encode image: {}", imageRes.unwrapErr());
    }
    if (auto res = file::writeBinary(m_pendingPath, imageRes.unwrap()); res.isErr()) {
        return Notify::error("Failed to save image: {}", res.unwrapErr());
    }

    auto name = MoreIcons::getText(m_nameInput);

    if (auto icon = more_icons::getIcon(name, IconType::Special)) {
        more_icons::updateIcon(icon);
    }
    else {
        auto jsonPath = Filesystem::withExt(m_pendingPath, L(".json"));
        (void)file::writeString(jsonPath, "{}");
        icon = more_icons::addTrail(
            name, name, std::move(m_pendingPath), std::move(jsonPath), {}, {}, "More Icons", 0, Defaults::getTrailInfo(0), false, false
        );
        if (Icons::preloadIcons) Icons::createAndAddFrames(icon);
    }

    auto notif = fmt::format("{} saved!", name);

    close();
    m_parentPopup->close();

    Notify::success(notif);
    MoreIcons::updateGarage();
}

void EditTrailPopup::onClose(CCObject* sender) {
    if (!m_hasChanged && MoreIcons::getText(m_nameInput).empty()) return close();
    auto alert = FLAlertLayer::create(
        this,
        "Exit Trail Editor",
        "Are you sure you want to <cy>exit</c> the <cg>trail editor</c>?\n<cr>All unsaved changes will be lost!</c>",
        "No",
        "Yes",
        350.0f
    );
    alert->setTag(1);
    alert->show();
}

void EditTrailPopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (!btn2) return;
    switch (layer->getTag()) {
        case 0: saveTrail(); break;
        case 1: close(); break;
    }
}
