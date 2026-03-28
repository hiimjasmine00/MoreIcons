#include "EditDeathEffectPopup.hpp"
#include "../FramePresetPopup.hpp"
#include "../IconButton.hpp"
#include "../IconPresetPopup.hpp"
#include "../ImageRenderer.hpp"
#include "../LoadEditorPopup.hpp"
#include "../SaveIconPopup.hpp"
#include "../../../misc/MultiControl.hpp"
#include "../../../../MoreIcons.hpp"
#include "../../../../utils/Defaults.hpp"
#include "../../../../utils/Filesystem.hpp"
#include "../../../../utils/Get.hpp"
#include "../../../../utils/Json.hpp"
#include "../../../../utils/Load.hpp"
#include "../../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/utils/file.hpp>
#include <jasmine/mod.hpp>
#include <matjson/std.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;
using namespace jasmine::mod;
using namespace std::string_literals;

EditDeathEffectPopup* EditDeathEffectPopup::create() {
    auto ret = new EditDeathEffectPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool EditDeathEffectPopup::init() {
    if (!BasePopup::init(450.0f, 280.0f, "geode.loader/GE_square03.png", CircleBaseColor::DarkPurple)) return false;

    setID("EditDeathEffectPopup");
    setTitle("Death Effect Editor");
    m_title->setID("edit-death-effect-title");

    auto previewBackground = NineSlice::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    previewBackground->setPosition({ 51.0f, 205.0f });
    previewBackground->setContentSize({ 80.0f, 80.0f });
    previewBackground->setOpacity(105);
    previewBackground->setID("preview-background");
    m_mainLayer->addChild(previewBackground);

    auto piecesBackground = NineSlice::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    piecesBackground->setPosition({ 270.0f, 222.0f });
    piecesBackground->setContentSize({ 260.0f, 45.0f });
    piecesBackground->setOpacity(105);
    piecesBackground->setID("pieces-background");
    m_mainLayer->addChild(piecesBackground);

    auto controlBackground = NineSlice::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    controlBackground->setPosition({ 270.0f, 122.0f });
    controlBackground->setContentSize({ 345.0f, 150.0f });
    controlBackground->setOpacity(50);
    controlBackground->setID("control-background");
    m_mainLayer->addChild(controlBackground);

    m_pieceMenu = CCMenu::create();
    m_pieceMenu->setPosition({ 270.0f, 222.0f });
    m_pieceMenu->setContentSize({ 260.0f, 45.0f });
    m_pieceMenu->ignoreAnchorPointForPosition(false);
    m_pieceMenu->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even), false);
    m_pieceMenu->setID("piece-menu");
    m_mainLayer->addChild(m_pieceMenu);

    m_previewNode = CCNode::create();
    m_previewNode->setPosition({ 51.0f, 205.0f });
    m_previewNode->setID("effect-preview-node");
    m_mainLayer->addChild(m_previewNode);

    m_previewSprite = CCSprite::create();
    m_previewSprite->setID("effect-preview-sprite");
    m_previewNode->addChild(m_previewSprite);

    auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    prevSprite->setScale(0.8f);
    auto prevButton = CCMenuItemSpriteExtra::create(prevSprite, this, menu_selector(EditDeathEffectPopup::onPrevPage));
    prevButton->setPosition({ 120.0f, 222.0f });
    prevButton->setID("prev-button");
    m_buttonMenu->addChild(prevButton);

    auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextSprite->setScale(0.8f);
    nextSprite->setFlipX(true);
    auto nextButton = CCMenuItemSpriteExtra::create(nextSprite, this, menu_selector(EditDeathEffectPopup::onNextPage));
    nextButton->setPosition({ 420.0f, 222.0f });
    nextButton->setID("next-button");
    m_buttonMenu->addChild(nextButton);

    auto saveMenu = CCMenu::create();
    saveMenu->setPosition({ 51.0f, 110.0f });
    saveMenu->setContentSize({ 30.0f, 100.0f });
    saveMenu->ignoreAnchorPointForPosition(false);
    saveMenu->setID("save-menu");
    m_mainLayer->addChild(saveMenu);

    auto loadStateSprite = CircleButtonSprite::createWithSprite("MI_loadBtn_001.png"_spr, 1.0f, CircleBaseColor::Green, CircleBaseSize::Small);
    loadStateSprite->setScale(0.7f);
    auto loadStateButton = CCMenuItemSpriteExtra::create(loadStateSprite, this, menu_selector(EditDeathEffectPopup::onLoadState));
    loadStateButton->setID("load-state-button");
    saveMenu->addChild(loadStateButton);

    auto saveStateSprite = CircleButtonSprite::createWithSprite("MI_saveBtn_001.png"_spr, 1.0f, CircleBaseColor::Cyan, CircleBaseSize::Small);
    saveStateSprite->setScale(0.7f);
    auto saveStateButton = CCMenuItemSpriteExtra::create(saveStateSprite, this, menu_selector(EditDeathEffectPopup::onSaveState));
    saveStateButton->setID("save-state-button");
    saveMenu->addChild(saveStateButton);

    saveMenu->setLayout(ColumnLayout::create()->setGap(10.0f)->setAxisReverse(true));

    m_selectSprite = CCSprite::createWithSpriteFrameName("GJ_select_001.png");
    m_selectSprite->setID("select-sprite");
    m_mainLayer->addChild(m_selectSprite);

    createControls({ 185.0f, 175.0f }, "Offset X:", "offset-x-control", 0);
    createControls({ 355.0f, 175.0f }, "Offset Y:", "offset-y-control", 1);
    createControls({ 185.0f, 135.0f }, "Rotation X:", "rotation-x-control", 2);
    createControls({ 355.0f, 135.0f }, "Rotation Y:", "rotation-y-control", 3);
    createControls({ 185.0f, 95.0f }, "Scale X:", "scale-x-control", 4);
    createControls({ 355.0f, 95.0f }, "Scale Y:", "scale-y-control", 5);

    auto pieceButtonMenu = CCMenu::create();
    pieceButtonMenu->setPosition({ 270.0f, 60.0f });
    pieceButtonMenu->setContentSize({ 330.0f, 20.0f });
    pieceButtonMenu->ignoreAnchorPointForPosition(false);
    pieceButtonMenu->setID("piece-button-menu");
    m_mainLayer->addChild(pieceButtonMenu);

    auto pieceAddSprite = ButtonSprite::create("Add", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    pieceAddSprite->setScale(0.6f);
    auto pieceAddButton = CCMenuItemSpriteExtra::create(pieceAddSprite, this, menu_selector(EditDeathEffectPopup::onPieceAdd));
    pieceAddButton->setID("piece-add-button");
    pieceButtonMenu->addChild(pieceAddButton);

    auto pieceImportSprite = ButtonSprite::create("Import", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    pieceImportSprite->setScale(0.6f);
    auto pieceImportButton = CCMenuItemSpriteExtra::create(pieceImportSprite, this, menu_selector(EditDeathEffectPopup::onPieceImport));
    pieceImportButton->setID("piece-import-button");
    pieceButtonMenu->addChild(pieceImportButton);

    auto piecePresetSprite = ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    piecePresetSprite->setScale(0.6f);
    auto piecePresetButton = CCMenuItemSpriteExtra::create(piecePresetSprite, this, menu_selector(EditDeathEffectPopup::onPiecePreset));
    piecePresetButton->setID("piece-preset-button");
    pieceButtonMenu->addChild(piecePresetButton);

    auto pieceRemoveSprite = ButtonSprite::create("Remove", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    pieceRemoveSprite->setScale(0.6f);
    auto pieceRemoveButton = CCMenuItemSpriteExtra::create(pieceRemoveSprite, this, menu_selector(EditDeathEffectPopup::onPieceRemove));
    pieceRemoveButton->setID("piece-remove-button");
    pieceButtonMenu->addChild(pieceRemoveButton);

    pieceButtonMenu->setLayout(RowLayout::create()->setGap(30.0f));

    auto iconButtonMenu = CCMenu::create();
    iconButtonMenu->setPosition({ 225.0f, 30.0f });
    iconButtonMenu->setContentSize({ 450.0f, 30.0f });
    iconButtonMenu->ignoreAnchorPointForPosition(false);
    iconButtonMenu->setID("icon-button-menu");
    m_mainLayer->addChild(iconButtonMenu);

    m_pngSprite = ButtonSprite::create("PNG", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto pngButton = CCMenuItemSpriteExtra::create(m_pngSprite, this, menu_selector(EditDeathEffectPopup::onPNG));
    pngButton->setID("png-button");
    iconButtonMenu->addChild(pngButton);

    m_plistSprite = ButtonSprite::create("Plist", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto plistButton = CCMenuItemSpriteExtra::create(m_plistSprite, this, menu_selector(EditDeathEffectPopup::onPlist));
    plistButton->setID("plist-button");
    iconButtonMenu->addChild(plistButton);

    auto presetButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png", 0.8f), this, menu_selector(EditDeathEffectPopup::onPreset)
    );
    presetButton->setID("preset-button");
    iconButtonMenu->addChild(presetButton);

    auto saveButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f), this, menu_selector(EditDeathEffectPopup::onSave)
    );
    saveButton->setID("save-button");
    iconButtonMenu->addChild(saveButton);

    iconButtonMenu->setLayout(RowLayout::create()->setGap(30.0f));

    m_iconButton = IconButton::create();
    m_iconButton->setID("icon-button");
    m_buttonMenu->addChild(m_iconButton);

    MoreIcons::getIconPaths(nullptr, 2, IconType::DeathEffect, m_selectedPNG, m_selectedPlist);
    updateWithSelectedFiles();
    m_hasChanged = false;

    handleTouchPriority(this);

    return true;
}

void EditDeathEffectPopup::onPrevPage(CCObject* sender) {
    if (m_frames.empty()) return;
    if (m_page <= 0) m_page = (m_frames.size() - 1) / 5;
    else m_page--;
    updateState();
}

void EditDeathEffectPopup::onNextPage(CCObject* sender) {
    if (m_frames.empty()) return;
    if (m_page >= (m_frames.size() - 1) / 5) m_page = 0;
    else m_page++;
    updateState();
}

void EditDeathEffectPopup::onPieceAdd(CCObject* sender) {
    m_listener.spawn(file::pick(file::PickMode::OpenFile, {
        .filters = {{
            .description = "PNG files",
            .files = { "*.png" }
        }}
    }), [this](Result<std::optional<std::filesystem::path>> res) {
        if (res.isErr()) return Notify::error("Failed to import PNG file: {}", res.unwrapErr());

        auto path = std::move(res).unwrap();
        if (!path.has_value()) return;

        if (auto textureRes = Load::createTexture(path.value())) {
            auto frame = MoreIcons::frameWithTexture(textureRes.unwrap());
            auto button = addPieceButton(m_frames.size(), frame);
            m_frames.push_back(frame);
            m_page = (m_frames.size() - 1) / 5;
            updatePieces();
            onSelectPiece(button);
        }
        else if (textureRes.isErr()) return Notify::error(textureRes.unwrapErr());
    });
}

void EditDeathEffectPopup::onPieceImport(CCObject* sender) {
    if (m_frames.empty()) return onPieceAdd(sender);
    m_listener.spawn(file::pick(file::PickMode::OpenFile, {
        .filters = {{
            .description = "PNG files",
            .files = { "*.png" }
        }}
    }), [this](Result<std::optional<std::filesystem::path>> res) {
        if (res.isErr()) return Notify::error("Failed to import PNG file: {}", res.unwrapErr());

        auto path = std::move(res).unwrap();
        if (!path.has_value()) return;

        if (auto textureRes = Load::createTexture(path.value())) {
            m_frames[m_selectedPiece] = MoreIcons::frameWithTexture(textureRes.unwrap());
            updatePieces();
            onSelectPiece(m_pieceButtons[m_selectedPiece]);
        }
        else if (textureRes.isErr()) return Notify::error(textureRes.unwrapErr());
    });
}

void EditDeathEffectPopup::onPiecePreset(CCObject* sender) {
    FramePresetPopup::create(IconType::DeathEffect, [this](CCSpriteFrame* frame) {
        m_frames[m_selectedPiece] = frame;
        updatePieces();
        onSelectPiece(m_pieceButtons[m_selectedPiece]);
    })->show();
}

void EditDeathEffectPopup::onPieceRemove(CCObject* sender) {
    if (m_frames.empty()) return;
    m_frames.erase(m_frames.begin() + m_selectedPiece);
    m_pieceButtons[m_selectedPiece]->removeFromParent();
    m_pieceButtons.erase(m_pieceButtons.begin() + m_selectedPiece);
    m_pieces.erase(m_pieces.begin() + m_selectedPiece);
    m_definitions.erase(m_definitions.begin() + m_selectedPiece);
    if (m_selectedPiece >= m_frames.size()) m_selectedPiece = m_frames.size() - 1;
    updatePieces();
    for (size_t i = 0; i < m_frames.size(); i++) {
        m_pieceButtons[i]->setTag(i);
        m_pieceButtons[i]->setID(fmt::format("piece-{}-button", i + 1));
    }
    if (!m_frames.empty()) onSelectPiece(m_pieceButtons[m_selectedPiece]);
    else MoreIcons::setTexture(m_previewSprite, nullptr);
}

void EditDeathEffectPopup::onLoadState(CCObject* sender) {
    LoadEditorPopup::create(IconType::DeathEffect, [this](const std::filesystem::path& directory) {
        return loadEditor(directory);
    })->show();
}

Result<> EditDeathEffectPopup::loadEditor(const std::filesystem::path& directory) {
    GEODE_UNWRAP_INTO(auto state, file::readJson(directory / L("state.json")));
    if (!state.isObject()) return Err("Expected object");

    m_selectedPNG = directory / L("effect.png");
    m_selectedPlist = directory / L("effect.plist");
    GEODE_UNWRAP(updateWithSelectedFilesInternal(false));

    auto iconPath = directory / L("icon.png");
    if (Filesystem::doesExist(iconPath)) {
        m_iconButton->setIcon(iconPath);
    }
    else {
        m_iconButton->resetIcon();
    }

    m_definitions = Json::get<std::vector<FrameDefinition>>(state, "definitions");
    if (m_selectedPiece >= m_definitions.size()) m_selectedPiece = m_definitions.size() - 1;
    m_definition = &m_definitions[m_selectedPiece];
    updateControls();
    updatePieces();
    onSelectPiece(m_pieceButtons[m_selectedPiece]);

    return Ok();
}

void EditDeathEffectPopup::onSaveState(CCObject* sender) {
    SaveIconPopup::create(
        IconType::DeathEffect, true,
        [this](const gd::string& name) {
            m_pendingPath = MoreIcons::getEditorDir(IconType::DeathEffect) / Filesystem::strWide(name);
            return Filesystem::doesExist(m_pendingPath);
        },
        [this](const gd::string& name) {
            return saveEditor();
        },
        [this] {
            m_pendingPath.clear();
        }
    )->show();
}

Result<> EditDeathEffectPopup::saveEditor() {
    if (!Filesystem::doesExist(m_pendingPath)) {
        GEODE_UNWRAP(file::createDirectoryAll(m_pendingPath));
    }

    GEODE_UNWRAP(file::writeString(m_pendingPath / L("state.json"), matjson::makeObject({
        { "definitions"s, matjson::Value(m_definitions) }
    }).dump()).mapErr([](std::string err) {
        return fmt::format("Failed to save state: {}", err);
    }));

    texpack::Packer packer;
    for (size_t i = 0; i < m_frames.size(); ++i) {
        auto sprite = CCSprite::createWithSpriteFrame(m_frames[i]);
        sprite->setAnchorPoint({ 0.0f, 0.0f });
        sprite->setBlendFunc({ GL_ONE, GL_ZERO });
        packer.frame(fmt::format("effect_{:03}.png", i + 1), ImageRenderer::getImage(sprite));
        sprite->release();
    }

    GEODE_UNWRAP(ImageRenderer::save(packer, m_pendingPath / L("effect.png"), m_pendingPath / L("effect.plist"), "effect.png"));

    if (auto texture = m_iconButton->getIconTexture()) {
        auto iconSprite = CCSprite::createWithTexture(texture);
        iconSprite->setAnchorPoint({ 0.0f, 0.0f });
        iconSprite->setBlendFunc({ GL_ONE, GL_ZERO });
        auto iconImage = ImageRenderer::getImage(iconSprite);
        iconSprite->release();

        GEODE_UNWRAP_INTO(auto iconData, texpack::toPNG(iconImage).mapErr([](std::string err) {
            return fmt::format("Failed to encode icon: {}", err);
        }));

        GEODE_UNWRAP(file::writeBinary(m_pendingPath / L("icon.png"), iconData).mapErr([](std::string err) {
            return fmt::format("Failed to save icon: {}", err);
        }));
    }

    m_hasChanged = false;
    return Ok();
}

void EditDeathEffectPopup::onPNG(CCObject* sender) {
    m_listener.spawn(file::pick(file::PickMode::OpenFile, {
        .filters = {{
            .description = "PNG files",
            .files = { "*.png" }
        }}
    }), [this](Result<std::optional<std::filesystem::path>> res) {
        if (res.isErr()) return Notify::error("Failed to import PNG file: {}", res.unwrapErr());

        auto path = std::move(res).unwrap();
        if (!path.has_value()) return;

        m_selectedPNG = std::move(path).value();
        if (m_selectedPlist.empty()) {
            m_pngSprite->updateBGImage("GJ_button_03.png");
            m_plistSprite->m_BGSprite->runAction(CCRepeat::create(CCSequence::createWithTwoActions(
                CCEaseIn::create(CCTintTo::create(0.4f, 127, 255, 127), 2.0f),
                CCEaseOut::create(CCTintTo::create(0.4f, 255, 255, 255), 2.0f)
            ), 2));
            m_hasChanged = true;
        }
        else {
            m_plistSprite->updateBGImage("GJ_button_05.png");
            updateWithSelectedFiles();
        }
    });
}

void EditDeathEffectPopup::onPlist(CCObject* sender) {
    m_listener.spawn(file::pick(file::PickMode::OpenFile, {
        .filters = {{
            .description = "Plist files",
            .files = { "*.plist" }
        }}
    }), [this](Result<std::optional<std::filesystem::path>> res) {
        if (res.isErr()) return Notify::error("Failed to import Plist file: {}", res.unwrapErr());

        auto path = std::move(res).unwrap();
        if (!path.has_value()) return;

        m_selectedPlist = std::move(path).value();
        if (m_selectedPNG.empty()) {
            m_plistSprite->updateBGImage("GJ_button_03.png");
            m_pngSprite->m_BGSprite->runAction(CCRepeat::create(CCSequence::createWithTwoActions(
                CCEaseIn::create(CCTintTo::create(0.4f, 127, 255, 127), 2.0f),
                CCEaseOut::create(CCTintTo::create(0.4f, 255, 255, 255), 2.0f)
            ), 2));
            m_hasChanged = true;
        }
        else {
            m_pngSprite->updateBGImage("GJ_button_05.png");
            updateWithSelectedFiles();
        }
    });
}

void EditDeathEffectPopup::onPreset(CCObject* sender) {
    IconPresetPopup::create(IconType::DeathEffect, {}, [this](int id, IconInfo* info) {
        MoreIcons::getIconPaths(info, id, IconType::DeathEffect, m_selectedPNG, m_selectedPlist);
        updateWithSelectedFiles();
    })->show();
}

void EditDeathEffectPopup::onSave(CCObject* sender) {
    if (m_frames.empty()) return Notify::error("No pieces to save.");

    SaveIconPopup::create(
        IconType::DeathEffect, false,
        [this](const gd::string& name) {
            m_pendingPath = MoreIcons::getIconStem(name, IconType::DeathEffect);
            return Filesystem::doesExist(m_pendingPath);
        },
        [this](const gd::string& name) {
            return saveEffect(name);
        },
        [this] {
            m_pendingPath.clear();
        }
    )->show();
}

Result<> EditDeathEffectPopup::saveEffect(const gd::string& name) {
    if (!Filesystem::doesExist(m_pendingPath)) {
        GEODE_UNWRAP(file::createDirectoryAll(m_pendingPath));
    }

    std::array<texpack::Packer, 3> packers = {};
    std::array pngs = {
        m_pendingPath / L("effect-uhd.png"),
        m_pendingPath / L("effect-hd.png"),
        m_pendingPath / L("effect.png")
    };
    std::array plists = {
        m_pendingPath / L("effect-uhd.plist"),
        m_pendingPath / L("effect-hd.plist"),
        m_pendingPath / L("effect.plist")
    };
    auto scaleFactor = Get::director->getContentScaleFactor();
    int index;
    if (scaleFactor >= 4.0f) index = 0;
    else if (scaleFactor >= 2.0f) index = 1;
    else index = 2;
    std::array scales = { 4.0f / scaleFactor, 2.0f / scaleFactor, 1.0f / scaleFactor };
    for (size_t i = 0; i < m_frames.size(); i++) {
        auto joinedName = fmt::format("effect_{:03}.png", i + 1);
        auto frame = m_frames[i].data();
        auto& definition = m_definitions[i];
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

    GEODE_UNWRAP(ImageRenderer::save(packers[0], pngs[0], plists[0], "effect-uhd.png").mapErr([](std::string err) {
        return fmt::format("Failed to save UHD effect: {}", err);
    }));

    GEODE_UNWRAP(ImageRenderer::save(packers[1], pngs[1], plists[1], "effect-hd.png").mapErr([](std::string err) {
        return fmt::format("Failed to save HD effect: {}", err);
    }));

    GEODE_UNWRAP(ImageRenderer::save(packers[2], pngs[2], plists[2], "effect.png").mapErr([](std::string err) {
        return fmt::format("Failed to save SD effect: {}", err);
    }));

    auto iconPath = m_iconButton->saveIcon(m_pendingPath);

    if (auto icon = more_icons::getIcon(name, IconType::DeathEffect)) {
        if (!iconPath.empty() && icon->getIcon().empty()) {
            icon->setIcon(std::move(iconPath));
        }
        more_icons::updateIcon(icon);
    }
    else {
        auto jsonPath = m_pendingPath / L("settings.json");
        (void)file::writeString(jsonPath, Defaults::getDeathEffectInfo(0).dump());
        more_icons::addDeathEffect(name, name, std::move(pngs[index]), std::move(plists[index]),
            std::move(jsonPath), std::move(iconPath), Get::director->getLoadedTextureQuality());
    }

    return Ok();
}

void EditDeathEffectPopup::createControls(const CCPoint& pos, const char* text, std::string&& id, int offset) {
    auto def = offset == 4 || offset == 5 ? 1.0f : 0.0f;
    auto min = offset == 0 || offset == 1 ? -20.0f : offset == 4 || offset == 5 ? -10.0f : 0.0f;
    auto max = offset == 0 || offset == 1 ? 20.0f : offset == 4 || offset == 5 ? 10.0f : 360.0f;

    auto multiControl = MultiControl::create([this, offset](float value) {
        reinterpret_cast<float*>(m_definition)[offset] = value;
        updateTargets();
    }, text, def, min, max, def, 3, 0.75f, 80.0f);
    multiControl->setPosition(pos);
    multiControl->setID(std::move(id));
    m_mainLayer->addChild(multiControl);

    multiControl->getSlider()->setPosition({ 0.0f, -10.0f });
    multiControl->getLabel()->setScale(0.6f);
    multiControl->getInput()->setScale(0.5f);

    auto menu = multiControl->getMenu();
    menu->setPosition({ 0.0f, 10.0f });
    menu->updateLayout();

    m_controls[offset] = multiControl;
}

void EditDeathEffectPopup::updateControls() {
    m_controls[0]->setValue(m_definition->offsetX);
    m_controls[1]->setValue(m_definition->offsetY);
    m_controls[2]->setValue(m_definition->rotationX);
    m_controls[3]->setValue(m_definition->rotationY);
    m_controls[4]->setValue(m_definition->scaleX);
    m_controls[5]->setValue(m_definition->scaleY);
}

CCMenuItemSpriteExtra* EditDeathEffectPopup::addPieceButton(int index, CCSpriteFrame* frame) {
    m_definitions.emplace(m_definitions.begin() + index);

    if (!frame) frame = Get::spriteFrameCache->spriteFrameByName("GJ_deleteIcon_001.png");
    auto pieceSprite = CCSprite::createWithSpriteFrame(frame);
    limitNodeSize(pieceSprite, { 30.0f, 30.0f }, 1.0f, 0.0f);
    auto pieceButton = CCMenuItemSpriteExtra::create(pieceSprite, this, menu_selector(EditDeathEffectPopup::onSelectPiece));
    pieceSprite->setPosition({ 15.0f, 15.0f });
    pieceButton->setContentSize({ 30.0f, 30.0f });
    pieceButton->setTag(index);
    pieceButton->setID(fmt::format("piece-{}-button", index + 1));
    m_pieceMenu->addChild(pieceButton);

    m_pieces.push_back(pieceSprite);
    m_pieceButtons.push_back(pieceButton);
    return pieceButton;
}

void EditDeathEffectPopup::onSelectPiece(CCObject* sender) {
    auto node = static_cast<CCNode*>(sender);
    m_selectedPiece = node->getTag();
    m_definition = &m_definitions[m_selectedPiece];
    updateControls();
    m_selectSprite->setVisible(true);
    m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_pieceMenu->convertToWorldSpace(node->getPosition())));
    m_previewSprite->setDisplayFrame(m_frames[m_selectedPiece]);
    limitNodeSize(m_previewSprite, { 75.0f, 75.0f }, 1.0f, 0.0f);
    updateTargets();
}

void EditDeathEffectPopup::updateWithSelectedFiles(bool update) {
    if (auto res = updateWithSelectedFilesInternal(update); res.isErr()) {
        Notify::error(res.unwrapErr());
    }
}

Result<> EditDeathEffectPopup::updateWithSelectedFilesInternal(bool update) {
    auto imageRes = Load::createFrames(m_selectedPNG, m_selectedPlist, {}, IconType::DeathEffect);
    if (imageRes.isErr()) {
        m_selectedPNG.clear();
        m_selectedPlist.clear();
        return std::move(imageRes).asErr();
    }

    auto image = std::move(imageRes).unwrap();
    Load::initTexture(image.texture, image.data.data(), image.width, image.height);

    m_frames.clear();
    m_pieceButtons.clear();
    m_pieces.clear();
    m_definitions.clear();
    m_pieceMenu->removeAllChildren();

    std::vector<std::string_view> keys;
    for (auto& frame : image.frames) {
        keys.push_back(frame.first);
    }
    std::ranges::sort(keys);
    CCMenuItemSpriteExtra* selected = nullptr;
    for (size_t i = 0; i < keys.size(); ++i) {
        auto it = image.frames.find(keys[i]);
        if (it != image.frames.end()) {
            auto button = addPieceButton(m_frames.size(), it->second);
            if (i == m_selectedPiece) selected = button;
            m_frames.push_back(std::move(it->second));
            image.frames.erase(it);
        }
    }
    if (update) {
        if (!selected) selected = m_pieceButtons.back();

        updatePieces();
        onSelectPiece(selected);
    }

    m_selectedPNG.clear();
    m_selectedPlist.clear();
    return Ok();
}

void EditDeathEffectPopup::updatePieces() {
    auto crossFrame = Get::spriteFrameCache->spriteFrameByName("GJ_deleteIcon_001.png");
    for (size_t i = 0; i < m_frames.size(); ++i) {
        auto spriteFrame = m_frames[i].data();
        m_pieces[i]->setDisplayFrame(spriteFrame ? spriteFrame : crossFrame);
        limitNodeSize(m_pieces[i], { 30.0f, 30.0f }, 1.0f, 0.0f);
    }

    updateState();

    m_hasChanged = true;
}

void EditDeathEffectPopup::updateState() {
    m_selectSprite->setVisible(m_selectedPiece >= m_page * 5 && m_selectedPiece < (m_page + 1) * 5);
    for (size_t i = 0; i < m_pieceButtons.size(); i++) {
        m_pieceButtons[i]->setVisible(i >= m_page * 5 && i < (m_page + 1) * 5);
    }
    m_pieceMenu->updateLayout();
}

void EditDeathEffectPopup::updateTargets() {
    m_hasChanged = true;

    if (m_definition) {
        m_previewNode->setPositionX(m_definition->offsetX + 51.0f);
        m_previewNode->setPositionY(m_definition->offsetY + 205.0f);
        m_previewNode->setRotationX(m_definition->rotationX);
        m_previewNode->setRotationY(m_definition->rotationY);
        m_previewNode->setScaleX(m_definition->scaleX);
        m_previewNode->setScaleY(m_definition->scaleY);
    }
}

void EditDeathEffectPopup::onClose(CCObject* sender) {
    if (!m_hasChanged) return close();

    FLAlertLayer::create(
        this,
        "Exit Death Effect Editor",
        "Are you sure you want to <cy>exit</c> the <cg>death effect editor</c>?\n<cr>All unsaved changes will be lost!</c>",
        "No",
        "Yes",
        350.0f
    )->show();
}

void EditDeathEffectPopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (btn2) close();
}
