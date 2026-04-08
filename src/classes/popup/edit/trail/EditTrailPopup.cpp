#include "EditTrailPopup.hpp"
#include "../IconButton.hpp"
#include "../IconPresetPopup.hpp"
#include "../ImageRenderer.hpp"
#include "../SaveIconPopup.hpp"
#include "../../../../MoreIcons.hpp"
#include "../../../../utils/Defaults.hpp"
#include "../../../../utils/Filesystem.hpp"
#include "../../../../utils/Get.hpp"
#include "../../../../utils/Icons.hpp"
#include "../../../../utils/Load.hpp"
#include "../../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/utils/file.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

EditTrailPopup* EditTrailPopup::create() {
    auto ret = new EditTrailPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool EditTrailPopup::init() {
    if (!BasePopup::init(350.0f, 130.0f, "geode.loader/GE_square03.png", CircleBaseColor::DarkPurple)) return false;

    setID("EditTrailPopup");
    setTitle("Trail Editor");
    m_title->setID("edit-trail-title");

    m_streak = CCSprite::create();
    m_streak->setPosition({ 175.0f, 75.0f });
    m_streak->setRotation(-90.0f);
    m_streak->setID("streak-preview");
    m_mainLayer->addChild(m_streak);

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

    m_iconButton = IconButton::create();
    m_iconButton->setID("icon-button");
    m_buttonMenu->addChild(m_iconButton);

    updateWithPath(MoreIcons::getIconPath(nullptr, 1, IconType::Special));
    m_hasChanged = false;

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
    SaveIconPopup::create(
        IconType::Special, false,
        [this](const gd::string& name) {
            m_pendingPath = MoreIcons::getIconStem(name, IconType::Special);
            return Filesystem::doesExist(m_pendingPath);
        },
        [this](const gd::string& name) {
            return saveTrail(name);
        },
        [this] {
            m_pendingPath.clear();
        }
    )->show();
}

void EditTrailPopup::updateWithPath(const std::filesystem::path& path) {
    if (auto textureRes = Load::createTexture(path)) {
        MoreIcons::setTexture(m_streak, textureRes.unwrap());
        auto& size = m_streak->getContentSize();
        m_streak->setScaleX(14.0f / size.width);
        m_streak->setScaleY(160.0f / size.height);
        m_hasChanged = true;
    }
    else if (textureRes.isErr()) Notify::error(textureRes.unwrapErr());
}

Result<> EditTrailPopup::saveTrail(const gd::string& name) {
    if (!Filesystem::doesExist(m_pendingPath)) {
        GEODE_UNWRAP(file::createDirectoryAll(m_pendingPath));
    }

    GEODE_UNWRAP_INTO(auto imageData, ImageRenderer::getImage(m_streak->getTexture()).mapErr([](std::string err) {
        return fmt::format("Failed to encode image: {}", err);
    }));

    auto trailPath = m_pendingPath / L("trail.png");
    GEODE_UNWRAP(file::writeBinary(trailPath, imageData).mapErr([](std::string err) {
        return fmt::format("Failed to save image: {}", err);
    }));

    auto iconPath = m_iconButton->saveIcon(m_pendingPath);

    if (auto icon = more_icons::getIcon(name, IconType::Special)) {
        if (!iconPath.empty() && icon->getIcon().empty()) {
            icon->setIcon(std::move(iconPath));
        }
        more_icons::updateIcon(icon);
    }
    else {
        auto jsonPath = m_pendingPath / L("settings.json");
        (void)file::writeString(jsonPath, Defaults::getTrailInfo(0).dump());
        more_icons::addTrail(name, name, std::move(trailPath), std::move(jsonPath), std::move(iconPath));
    }

    return Ok();
}

void EditTrailPopup::onClose(CCObject* sender) {
    if (!m_hasChanged) return close();
    FLAlertLayer::create(
        this,
        "Exit Trail Editor",
        "Are you sure you want to <cy>exit</c> the <cg>trail editor</c>?\n<cr>All unsaved changes will be lost!</c>",
        "No",
        "Yes",
        350.0f
    )->show();
}

void EditTrailPopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (btn2) close();
}
