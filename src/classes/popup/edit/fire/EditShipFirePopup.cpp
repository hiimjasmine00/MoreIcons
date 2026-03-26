#include "EditShipFirePopup.hpp"
#include "../FramePresetPopup.hpp"
#include "../IconButton.hpp"
#include "../IconPresetPopup.hpp"
#include "../ImageRenderer.hpp"
#include "../SaveIconPopup.hpp"
#include "../../../../MoreIcons.hpp"
#include "../../../../utils/Defaults.hpp"
#include "../../../../utils/Filesystem.hpp"
#include "../../../../utils/Icons.hpp"
#include "../../../../utils/Load.hpp"
#include "../../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/utils/file.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

EditShipFirePopup* EditShipFirePopup::create() {
    auto ret = new EditShipFirePopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool EditShipFirePopup::init() {
    if (!BasePopup::init(400.0f, 160.0f, "geode.loader/GE_square03.png", CircleBaseColor::DarkPurple)) return false;

    setID("EditShipFirePopup");
    setTitle("Ship Fire Editor");
    m_title->setID("edit-ship-fire-title");

    m_streak = CCSprite::create();
    m_streak->setPosition({ 200.0f, 80.0f });
    m_streak->setRotation(-90.0f);
    m_streak->setID("streak-preview");
    m_mainLayer->addChild(m_streak);

    auto framesBackground = NineSlice::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    framesBackground->setPosition({ 200.0f, 110.0f });
    framesBackground->setContentSize({ 300.0f, 30.0f });
    framesBackground->setOpacity(105);
    framesBackground->setID("frames-background");
    m_mainLayer->addChild(framesBackground);

    m_frameMenu = CCMenu::create();
    m_frameMenu->setPosition({ 200.0f, 110.0f });
    m_frameMenu->setContentSize({ 300.0f, 30.0f });
    m_frameMenu->ignoreAnchorPointForPosition(false);
    m_frameMenu->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even), false);
    m_frameMenu->setID("frame-menu");
    m_mainLayer->addChild(m_frameMenu);

    m_selectSprite = CCSprite::createWithSpriteFrameName("GJ_select_001.png");
    m_selectSprite->setID("select-sprite");
    m_mainLayer->addChild(m_selectSprite);

    auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    prevSprite->setScale(0.5f);
    auto prevButton = CCMenuItemSpriteExtra::create(prevSprite, this, menu_selector(EditShipFirePopup::onPrev));
    prevButton->setPosition({ 40.0f, 110.0f });
    prevButton->setID("prev-button");
    m_buttonMenu->addChild(prevButton);

    auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextSprite->setScale(0.5f);
    nextSprite->setFlipX(true);
    auto nextButton = CCMenuItemSpriteExtra::create(nextSprite, this, menu_selector(EditShipFirePopup::onNext));
    nextButton->setPosition({ 360.0f, 110.0f });
    nextButton->setID("next-button");
    m_buttonMenu->addChild(nextButton);

    auto frameButtonMenu = CCMenu::create();
    frameButtonMenu->setPosition({ 200.0f, 55.0f });
    frameButtonMenu->setContentSize({ 400.0f, 20.0f });
    frameButtonMenu->ignoreAnchorPointForPosition(false);
    frameButtonMenu->setID("frame-button-menu");
    m_mainLayer->addChild(frameButtonMenu);

    auto frameAddSprite = ButtonSprite::create("Add", "goldFont.fnt", "GJ_button_05.png");
    frameAddSprite->setScale(0.6f);
    auto frameAddButton = CCMenuItemSpriteExtra::create(frameAddSprite, this, menu_selector(EditShipFirePopup::onFrameAdd));
    frameAddButton->setID("frame-add-button");
    frameButtonMenu->addChild(frameAddButton);

    auto frameImportSprite = ButtonSprite::create("Import", "goldFont.fnt", "GJ_button_05.png");
    frameImportSprite->setScale(0.6f);
    auto frameImportButton = CCMenuItemSpriteExtra::create(frameImportSprite, this, menu_selector(EditShipFirePopup::onFrameImport));
    frameImportButton->setID("frame-import-button");
    frameButtonMenu->addChild(frameImportButton);

    auto framePresetSprite = ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png");
    framePresetSprite->setScale(0.6f);
    auto framePresetButton = CCMenuItemSpriteExtra::create(framePresetSprite, this, menu_selector(EditShipFirePopup::onFramePreset));
    framePresetButton->setID("frame-preset-button");
    frameButtonMenu->addChild(framePresetButton);

    auto frameRemoveSprite = ButtonSprite::create("Remove", "goldFont.fnt", "GJ_button_05.png");
    frameRemoveSprite->setScale(0.6f);
    auto frameRemoveButton = CCMenuItemSpriteExtra::create(frameRemoveSprite, this, menu_selector(EditShipFirePopup::onFrameRemove));
    frameRemoveButton->setID("frame-remove-button");
    frameButtonMenu->addChild(frameRemoveButton);

    frameButtonMenu->setLayout(RowLayout::create()->setGap(25.0f));

    auto bottomMenu = CCMenu::create();
    bottomMenu->setPosition({ 200.0f, 25.0f });
    bottomMenu->setContentSize({ 400.0f, 30.0f });
    bottomMenu->ignoreAnchorPointForPosition(false);
    bottomMenu->setID("bottom-menu");
    m_mainLayer->addChild(bottomMenu);

    auto importSprite = ButtonSprite::create("Import", "goldFont.fnt", "GJ_button_05.png");
    importSprite->setScale(0.9f);
    auto importButton = CCMenuItemSpriteExtra::create(importSprite, this, menu_selector(EditShipFirePopup::onImport));
    importButton->setID("import-button");
    bottomMenu->addChild(importButton);

    auto presetSprite = ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png");
    presetSprite->setScale(0.9f);
    auto presetButton = CCMenuItemSpriteExtra::create(presetSprite, this, menu_selector(EditShipFirePopup::onPreset));
    presetButton->setID("preset-button");
    bottomMenu->addChild(presetButton);

    auto saveSprite = ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png");
    saveSprite->setScale(0.9f);
    auto saveButton = CCMenuItemSpriteExtra::create(saveSprite, this, menu_selector(EditShipFirePopup::onSave));
    saveButton->setID("save-button");
    bottomMenu->addChild(saveButton);

    bottomMenu->setLayout(RowLayout::create()->setGap(20.0f));

    m_iconButton = IconButton::create();
    m_iconButton->setID("icon-button");
    m_buttonMenu->addChild(m_iconButton);

    updateWithPath(MoreIcons::getIconPath(nullptr, 3, IconType::ShipFire), 10);
    m_hasChanged = false;

    handleTouchPriority(this);

    return true;
}

CCMenuItemSpriteExtra* EditShipFirePopup::addFrameButton(CCTexture2D* texture) {
    auto index = m_frameButtons.size();
    auto frame = CCSprite::createWithTexture(texture, { { 0.0f, 0.0f }, texture ? texture->getContentSize() : CCSize { 0.0f, 0.0f }});
    limitNodeHeight(frame, 30.0f, 1.0f, 0.0f);
    auto button = CCMenuItemSpriteExtra::create(frame, this, menu_selector(EditShipFirePopup::onFrameSelect));
    button->setContentSize({ 30.0f, 30.0f });
    frame->setPosition({ 15.0f, 15.0f });
    button->setTag(index);
    button->setID(fmt::format("frame-{}-button", index + 1));
    m_frameMenu->addChild(button);
    m_frameButtons.push_back(button);
    return button;
}

void EditShipFirePopup::updateState() {
    m_selectSprite->setVisible(m_selectedFrame >= m_page * 5 && m_selectedFrame < (m_page + 1) * 5);
    for (size_t i = 0; i < m_frameButtons.size(); i++) {
        m_frameButtons[i]->setVisible(i >= m_page * 5 && i < (m_page + 1) * 5);
    }
    m_frameMenu->updateLayout();
}

void EditShipFirePopup::onPrev(CCObject* sender) {
    if (m_frameButtons.empty()) return;
    if (m_page <= 0) m_page = (m_frameButtons.size() - 1) / 5;
    else m_page--;
    updateState();
}

void EditShipFirePopup::onNext(CCObject* sender) {
    if (m_frameButtons.empty()) return;
    if (m_page >= (m_frameButtons.size() - 1) / 5) m_page = 0;
    else m_page++;
    updateState();
}

void EditShipFirePopup::onFrameSelect(CCObject* sender) {
    m_selectedFrame = sender->getTag();
    m_selectSprite->setVisible(true);
    m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_frameMenu->convertToWorldSpace(static_cast<CCNode*>(sender)->getPosition())));
    MoreIcons::setTexture(m_streak, static_cast<CCSprite*>(static_cast<CCMenuItemSpriteExtra*>(sender)->getNormalImage())->getTexture());
    auto& size = m_streak->getContentSize();
    m_streak->setScaleX(20.0f / size.width);
    m_streak->setScaleY(80.0f / size.height);
}

void EditShipFirePopup::onFrameAdd(CCObject* sender) {
    m_pickListener.spawn(file::pick(file::PickMode::OpenFile, {
        .filters = {{
            .description = "PNG files",
            .files = { "*.png" }
        }}
    }), [this](Result<std::optional<std::filesystem::path>> res) {
        if (res.isErr()) return Notify::error("Failed to import PNG file: {}", res.unwrapErr());

        auto path = std::move(res).unwrap();
        if (!path.has_value()) return;

        auto textureRes = Load::createTexture(path.value());
        if (textureRes.isErr()) return Notify::error("Failed to load image: {}", textureRes.unwrapErr());

        auto button = addFrameButton(textureRes.unwrap());
        m_page = (m_frameButtons.size() - 1) / 5;
        updateState();
        onFrameSelect(button);
        m_hasChanged = true;
    });
}

void EditShipFirePopup::onFrameImport(CCObject* sender) {
    if (m_frameButtons.empty()) return onFrameAdd(sender);
    m_pickListener.spawn(file::pick(file::PickMode::OpenFile, {
        .filters = {{
            .description = "PNG files",
            .files = { "*.png" }
        }}
    }), [this](Result<std::optional<std::filesystem::path>> res) {
        if (res.isErr()) return Notify::error("Failed to import PNG file: {}", res.unwrapErr());

        auto path = std::move(res).unwrap();
        if (!path.has_value()) return;

        auto textureRes = Load::createTexture(path.value());
        if (textureRes.isErr()) return Notify::error("Failed to load image: {}", textureRes.unwrapErr());

        auto texture = textureRes.unwrap();
        MoreIcons::setTexture(static_cast<CCSprite*>(m_frameButtons[m_selectedFrame]->getNormalImage()), texture);
        MoreIcons::setTexture(m_streak, texture);
        m_hasChanged = true;
    });
}

void EditShipFirePopup::onFramePreset(CCObject* sender) {
    FramePresetPopup::create(IconType::ShipFire, [this](CCSpriteFrame* frame) {
        auto texture = frame->getTexture();
        if (m_frameButtons.empty()) {
            auto button = addFrameButton(texture);
            m_page = (m_frameButtons.size() - 1) / 5;
            updateState();
            onFrameSelect(button);
        }
        else {
            MoreIcons::setTexture(static_cast<CCSprite*>(m_frameButtons[m_selectedFrame]->getNormalImage()), texture);
            MoreIcons::setTexture(m_streak, texture);
        }
        m_hasChanged = true;
    })->show();
}

void EditShipFirePopup::onFrameRemove(CCObject* sender) {
    if (m_frameButtons.empty()) return;

    m_frameButtons[m_selectedFrame]->removeFromParent();
    m_frameButtons.erase(m_frameButtons.begin() + m_selectedFrame);
    if (m_selectedFrame >= m_frameButtons.size()) m_selectedFrame = m_frameButtons.size() - 1;

    if (m_frameButtons.empty()) {
        m_selectedFrame = 0;
        m_frameMenu->updateLayout();
        MoreIcons::setTexture(m_streak, nullptr);
        m_selectSprite->setVisible(false);
        return;
    }

    m_page = m_selectedFrame / 5;
    updateState();
    for (size_t i = 0; i < m_frameButtons.size(); i++) {
        auto button = m_frameButtons[i];
        button->setTag(i);
        button->setID(fmt::format("frame-{}-button", i));
    }
    onFrameSelect(m_frameButtons[m_selectedFrame]);
    m_hasChanged = true;
}

void EditShipFirePopup::onImport(CCObject* sender) {
    m_pickManyListener.spawn(file::pickMany({
        .filters = {{
            .description = "PNG files",
            .files = { "*.png" }
        }}
    }), [this](Result<std::vector<std::filesystem::path>> res) {
        if (res.isErr()) return Notify::error("Failed to import PNG files: {}", res.unwrapErr());

        auto paths = std::move(res).unwrap();
        if (paths.empty()) return;

        m_frameMenu->removeAllChildren();
        m_frameButtons.clear();
        CCMenuItemSpriteExtra* selected = nullptr;
        for (size_t i = 0; i < paths.size(); i++) {
            auto& path = paths[i];
            auto textureRes = Load::createTexture(path);
            if (textureRes.isErr()) return Notify::error("Failed to load {}: {}", Filesystem::filenameFormat(path), textureRes.unwrapErr());
            auto button = addFrameButton(textureRes.unwrap());
            if (i == m_selectedFrame) selected = button;
        }
        if (!selected) {
            selected = m_frameButtons.back();
            m_selectedFrame = m_frameButtons.size() - 1;
        }

        m_page = m_selectedFrame / 5;
        updateState();
        onFrameSelect(selected);
        m_hasChanged = true;
    });
}

void EditShipFirePopup::onPreset(CCObject* sender) {
    IconPresetPopup::create(IconType::ShipFire, {}, [this](int id, IconInfo* info) {
        updateWithPath(MoreIcons::getIconPath(info, id, IconType::ShipFire), info ? info->getFireCount() : Defaults::getShipFireCount(id));
    })->show();
}

void EditShipFirePopup::updateWithPath(std::filesystem::path path, int count) {
    m_frameMenu->removeAllChildren();
    m_frameButtons.clear();

    auto& pathString = Filesystem::getPathString(path);
    CCMenuItemSpriteExtra* selected = nullptr;
    for (int i = 0; i < count; i++) {
        pathString.replace(pathString.size() - 7, 3, fmt::format(L("{:03}"), i + 1));
        auto textureRes = Load::createTexture(path);
        if (textureRes.isErr()) return Notify::error("Failed to load {}: {}", Filesystem::filenameFormat(path), textureRes.unwrapErr());
        auto button = addFrameButton(textureRes.unwrap());
        if (i == m_selectedFrame) selected = button;
    }
    if (!selected) {
        selected = m_frameButtons.back();
        m_selectedFrame = m_frameButtons.size() - 1;
    }

    m_page = m_selectedFrame / 5;
    updateState();
    onFrameSelect(selected);
    m_hasChanged = true;
}

void EditShipFirePopup::onSave(CCObject* sender) {
    if (m_frameButtons.empty()) return Notify::info("Please add at least one frame.");

    SaveIconPopup::create(
        IconType::ShipFire, false,
        [this](ZStringView name) {
            m_pendingPath = MoreIcons::getIconStem(name, IconType::ShipFire);
            return Filesystem::doesExist(m_pendingPath);
        },
        [this](ZStringView name) {
            return saveShipFire(name);
        },
        [this] {
            m_pendingPath.clear();
        }
    )->show();
}

Result<> EditShipFirePopup::saveShipFire(ZStringView name) {
    if (!Filesystem::doesExist(m_pendingPath)) {
        GEODE_UNWRAP(file::createDirectoryAll(m_pendingPath));
    }

    for (size_t i = 0; i < m_frameButtons.size(); i++) {
        auto sprite = CCSprite::createWithTexture(static_cast<CCSprite*>(m_frameButtons[i]->getNormalImage())->getTexture());
        sprite->setAnchorPoint({ 0.0f, 0.0f });
        sprite->setBlendFunc({ GL_ONE, GL_ZERO });
        auto image = ImageRenderer::getImage(sprite);
        sprite->release();
        GEODE_UNWRAP_INTO(auto imageData, texpack::toPNG(image).mapErr([i](std::string err) {
            return fmt::format("Failed to encode fire_{:03}.png: {}", i + 1, err);
        }));
        GEODE_UNWRAP(file::writeBinary(m_pendingPath / fmt::format(L("fire_{:03}.png"), i + 1), imageData).mapErr([](std::string err) {
            return fmt::format("Failed to save image: {}", err);
        }));
    }

    auto iconPath = m_iconButton->saveIcon(m_pendingPath);

    if (auto icon = more_icons::getIcon(name, IconType::ShipFire)) {
        if (!iconPath.empty() && icon->getIcon().empty()) {
            icon->setIcon(std::move(iconPath));
        }
        more_icons::updateIcon(icon);
    }
    else {
        auto jsonPath = m_pendingPath / L("settings.json");
        (void)file::writeString(jsonPath, Defaults::getShipFireInfo(0).dump());
        more_icons::addShipFire(name, name, m_pendingPath / L("fire_001.png"), std::move(jsonPath), std::move(iconPath), m_frameButtons.size());
    }

    return Ok();
}

void EditShipFirePopup::onClose(CCObject* sender) {
    if (!m_hasChanged) return close();
    FLAlertLayer::create(
        this,
        "Exit Ship Fire Editor",
        "Are you sure you want to <cy>exit</c> the <cg>ship fire editor</c>?\n<cr>All unsaved changes will be lost!</c>",
        "No",
        "Yes",
        350.0f
    )->show();
}

void EditShipFirePopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (btn2) close();
}
