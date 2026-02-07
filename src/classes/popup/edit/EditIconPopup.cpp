#include "EditIconPopup.hpp"
#include "IconColorPopup.hpp"
#include "IconPresetPopup.hpp"
#include "ImageRenderer.hpp"
#include "LoadEditorPopup.hpp"
#include "SaveEditorPopup.hpp"
#include "SaveIconPopup.hpp"
#include "../MoreIconsPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Icons.hpp"
#include "../../../utils/Load.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/utils/file.hpp>
#include <jasmine/convert.hpp>
#include <jasmine/mod.hpp>

using namespace geode::prelude;
using namespace jasmine::mod;

EditIconPopup* EditIconPopup::create(BasePopup* popup, IconType type) {
    auto ret = new EditIconPopup();
    if (ret->init(popup, type)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCSpriteFrame* frameWithTexture(CCTexture2D* texture) {
    return CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() });
}

bool EditIconPopup::init(BasePopup* popup, IconType type) {
    if (!BasePopup::init(450.0f, 280.0f, "geode.loader/GE_square03.png")) return false;

    setID("EditIconPopup");
    setTitle(fmt::format("{} Editor", Constants::getSingularUppercase(type)));
    m_title->setID("edit-icon-title");

    m_parentPopup = popup;
    m_iconType = type;

    auto isRobot = type == IconType::Robot || type == IconType::Spider;

    auto iconBackground = NineSlice::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    iconBackground->setPosition({ 51.0f, 205.0f });
    iconBackground->setContentSize({ 80.0f, 80.0f });
    iconBackground->setOpacity(105);
    iconBackground->setID("icon-background");
    m_mainLayer->addChild(iconBackground);

    auto piecesBackground = NineSlice::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    piecesBackground->setPosition({ 270.0f, 222.0f });
    piecesBackground->setContentSize({ isRobot ? 260.0f : 330.0f, 45.0f });
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
    m_pieceMenu->setContentSize({ isRobot ? 260.0f : 330.0f, 45.0f });
    m_pieceMenu->ignoreAnchorPointForPosition(false);
    m_pieceMenu->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even), false);
    m_pieceMenu->setID("piece-menu");
    m_mainLayer->addChild(m_pieceMenu);

    m_player = SimpleIcon::create(type, MoreIcons::getIconName(1, type));
    m_player->setPosition({ 51.0f, 205.0f });
    m_player->setScale(2.0f);
    m_player->setID("icon-preview");
    m_mainLayer->addChild(m_player);

    CCMenuItemSpriteExtra* selected;
    if (isRobot) {
        auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        prevSprite->setScale(0.8f);
        auto prevButton = CCMenuItemSpriteExtra::create(prevSprite, this, menu_selector(EditIconPopup::onPrevPage));
        prevButton->setPosition({ 120.0f, 222.0f });
        prevButton->setID("prev-button");
        m_buttonMenu->addChild(prevButton);

        auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        nextSprite->setScale(0.8f);
        nextSprite->setFlipX(true);
        auto nextButton = CCMenuItemSpriteExtra::create(nextSprite, this, menu_selector(EditIconPopup::onNextPage));
        nextButton->setPosition({ 420.0f, 222.0f });
        nextButton->setID("next-button");
        m_buttonMenu->addChild(nextButton);

        m_pages.resize(4);
        m_pages[0].reserve(4);
        m_pages[1].reserve(3);
        m_pages[2].reserve(3);
        m_pages[3].reserve(3);
        m_required.reserve(12);

        selected = addPieceButton("_01_001", 0);
        addPieceButton("_01_2_001", 0);
        addPieceButton("_01_glow_001", 0);
        addPieceButton("_01_extra_001", 0, false);
        addPieceButton("_02_001", 1);
        addPieceButton("_02_2_001", 1);
        addPieceButton("_02_glow_001", 1);
        addPieceButton("_03_001", 2);
        addPieceButton("_03_2_001", 2);
        addPieceButton("_03_glow_001", 2);
        addPieceButton("_04_001", 3);
        addPieceButton("_04_2_001", 3);
        addPieceButton("_04_glow_001", 3);
    }
    else {
        m_pages.resize(1);
        m_pages[0].reserve(type == IconType::Ufo ? 5 : 4);
        m_required.reserve(type == IconType::Ufo ? 4 : 3);

        selected = addPieceButton("_001", 0);
        addPieceButton("_2_001", 0);
        if (type == IconType::Ufo) addPieceButton("_3_001", 0);
        addPieceButton("_glow_001", 0);
        addPieceButton("_extra_001", 0, false);
    }

    m_colorMenu = CCMenu::create();
    m_colorMenu->setPosition({ 33.0f, 110.0f });
    m_colorMenu->setContentSize({ 30.0f, 100.0f });
    m_colorMenu->ignoreAnchorPointForPosition(false);
    m_colorMenu->setID("color-menu");
    m_mainLayer->addChild(m_colorMenu);

    m_mainColorSprite = addColorButton(1, "1", "main-color-button");
    m_secondaryColorSprite = addColorButton(2, "2", "secondary-color-button");
    m_glowColorSprite = addColorButton(3, "G", "glow-color-button");

    m_colorMenu->setLayout(ColumnLayout::create()->setGap(10.0f)->setAxisReverse(true));

    auto saveMenu = CCMenu::create();
    saveMenu->setPosition({ 69.0f, 110.0f });
    saveMenu->setContentSize({ 30.0f, 100.0f });
    saveMenu->ignoreAnchorPointForPosition(false);
    saveMenu->setID("save-menu");
    m_mainLayer->addChild(saveMenu);

    auto loadStateSprite = CircleButtonSprite::createWithSprite("MI_loadBtn_001.png"_spr, 1.0f, CircleBaseColor::Green, CircleBaseSize::Small);
    loadStateSprite->setScale(0.7f);
    auto loadStateButton = CCMenuItemSpriteExtra::create(loadStateSprite, this, menu_selector(EditIconPopup::onLoadState));
    loadStateButton->setID("load-state-button");
    saveMenu->addChild(loadStateButton);

    auto saveStateSprite = CircleButtonSprite::createWithSprite("MI_saveBtn_001.png"_spr, 1.0f, CircleBaseColor::Cyan, CircleBaseSize::Small);
    saveStateSprite->setScale(0.7f);
    auto saveStateButton = CCMenuItemSpriteExtra::create(saveStateSprite, this, menu_selector(EditIconPopup::onSaveState));
    saveStateButton->setID("save-state-button");
    saveMenu->addChild(saveStateButton);

    saveMenu->setLayout(ColumnLayout::create()->setGap(10.0f)->setAxisReverse(true));

    m_selectSprite = CCSprite::createWithSpriteFrameName("GJ_select_001.png");
    m_selectSprite->setID("select-sprite");
    m_mainLayer->addChild(m_selectSprite);

    createControls({ 185.0f, 175.0f }, "Offset X:", "offset-x", 0);
    createControls({ 355.0f, 175.0f }, "Offset Y:", "offset-y", 1);
    createControls({ 185.0f, 135.0f }, "Rotation X:", "rotation-x", 2);
    createControls({ 355.0f, 135.0f }, "Rotation Y:", "rotation-y", 3);
    createControls({ 185.0f, 95.0f }, "Scale X:", "scale-x", 4);
    createControls({ 355.0f, 95.0f }, "Scale Y:", "scale-y", 5);

    goToPage(0);
    onSelectPiece(selected);

    auto pieceButtonMenu = CCMenu::create();
    pieceButtonMenu->setPosition({ 270.0f, 60.0f });
    pieceButtonMenu->setContentSize({ 330.0f, 20.0f });
    pieceButtonMenu->ignoreAnchorPointForPosition(false);
    pieceButtonMenu->setID("piece-button-menu");
    m_mainLayer->addChild(pieceButtonMenu);

    auto pieceImportSprite = ButtonSprite::create("Import", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    pieceImportSprite->setScale(0.6f);
    auto pieceImportButton = CCMenuItemSpriteExtra::create(pieceImportSprite, this, menu_selector(EditIconPopup::onPieceImport));
    pieceImportButton->setID("piece-import-button");
    pieceButtonMenu->addChild(pieceImportButton);

    auto piecePresetSprite = ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    piecePresetSprite->setScale(0.6f);
    auto piecePresetButton = CCMenuItemSpriteExtra::create(piecePresetSprite, this, menu_selector(EditIconPopup::onPiecePreset));
    piecePresetButton->setID("piece-preset-button");
    pieceButtonMenu->addChild(piecePresetButton);

    auto pieceClearSprite = ButtonSprite::create("Clear", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    pieceClearSprite->setScale(0.6f);
    auto pieceClearButton = CCMenuItemSpriteExtra::create(pieceClearSprite, this, menu_selector(EditIconPopup::onPieceClear));
    pieceClearButton->setID("piece-clear-button");
    pieceButtonMenu->addChild(pieceClearButton);

    pieceButtonMenu->setLayout(RowLayout::create()->setGap(30.0f));

    auto iconButtonMenu = CCMenu::create();
    iconButtonMenu->setPosition({ 225.0f, 30.0f });
    iconButtonMenu->setContentSize({ 450.0f, 30.0f });
    iconButtonMenu->ignoreAnchorPointForPosition(false);
    iconButtonMenu->setID("icon-button-menu");
    m_mainLayer->addChild(iconButtonMenu);

    m_pngSprite = ButtonSprite::create("PNG", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto pngButton = CCMenuItemSpriteExtra::create(m_pngSprite, this, menu_selector(EditIconPopup::onPNG));
    pngButton->setID("png-button");
    iconButtonMenu->addChild(pngButton);

    m_plistSprite = ButtonSprite::create("Plist", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto plistButton = CCMenuItemSpriteExtra::create(m_plistSprite, this, menu_selector(EditIconPopup::onPlist));
    plistButton->setID("plist-button");
    iconButtonMenu->addChild(plistButton);

    auto presetButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png", 0.8f), this, menu_selector(EditIconPopup::onPreset)
    );
    presetButton->setID("preset-button");
    iconButtonMenu->addChild(presetButton);

    auto saveButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f), this, menu_selector(EditIconPopup::onSave)
    );
    saveButton->setID("save-button");
    iconButtonMenu->addChild(saveButton);

    iconButtonMenu->setLayout(RowLayout::create()->setGap(30.0f));

    handleTouchPriority(this);

    return true;
}

void EditIconPopup::onPrevPage(CCObject* sender) {
    goToPage(m_page - 1);
}

void EditIconPopup::onNextPage(CCObject* sender) {
    goToPage(m_page + 1);
}

void EditIconPopup::onLoadState(CCObject* sender) {
    LoadEditorPopup::create(m_iconType, [this](const std::filesystem::path& directory, std::string_view name) {
        m_selectedPNG = directory / L("icon.png");
        m_selectedPlist = directory / L("icon.plist");
        if (!updateWithSelectedFiles()) return;

        auto stateRes = file::readFromJson<IconEditorState>(directory / L("state.json"));
        if (stateRes.isErr()) return Notify::error("Failed to load {}: {}", name, stateRes.unwrapErr());

        m_state = std::move(stateRes).unwrap();
        updateColor(1, m_state.mainColor);
        updateColor(2, m_state.secondaryColor);
        updateColor(3, m_state.glowColor);
        updateControls();

        for (auto& [key, definition] : m_state.definitions) {
            for (auto target : m_player->getTargets(key)) {
                target->setPositionX(definition.offsetX);
                target->setPositionY(definition.offsetY);
                target->setRotationX(definition.rotationX);
                target->setRotationY(definition.rotationY);
                target->setScaleX(definition.scaleX);
                target->setScaleY(definition.scaleY);
            }
        }

        Notify::success("{} loaded!", name);
    })->show();
}

void EditIconPopup::onSaveState(CCObject* sender) {
    SaveEditorPopup::create(m_iconType, m_state, m_frames, [this] {
        m_hasChanged = false;
    })->show();
}

void EditIconPopup::addFrame(std::string_view key, Ref<CCSpriteFrame>&& frame) {
    if (auto it = m_frames.find(key); it != m_frames.end()) {
        it->second = std::move(frame);
    }
    else {
        m_frames.emplace(std::string(key), std::move(frame));
    }
}

void EditIconPopup::eraseFrame(std::string_view key) {
    if (auto it = m_frames.find(key); it != m_frames.end()) {
        m_frames.erase(it);
    }
}

void EditIconPopup::onPieceImport(CCObject* sender) {
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
            addFrame(m_suffix, frameWithTexture(textureRes.unwrap()));
            updatePieces();
        }
        else if (textureRes.isErr()) return Notify::error(textureRes.unwrapErr());
    });
}

void EditIconPopup::onPiecePreset(CCObject* sender) {
    IconPresetPopup::create(m_iconType, m_suffix, [this](int id, IconInfo* info) {
        MoreIcons::getIconPaths(info, id, m_iconType, m_selectedPNG, m_selectedPlist);
        updateWithSelectedFiles(true);
    })->show();
}

void EditIconPopup::onPieceClear(CCObject* sender) {
    if (m_suffix.ends_with("_extra_001")) {
        eraseFrame(m_suffix);
    }
    else {
        auto emptyFrame = Icons::getFrame("emptyFrame.png"_spr);
        if (!emptyFrame) {
            emptyFrame = frameWithTexture(Load::createTexture(nullptr, 0, 0));
            Get::SpriteFrameCache()->addSpriteFrame(emptyFrame, "emptyFrame.png"_spr);
        }
        addFrame(m_suffix, emptyFrame);
    }
    updatePieces();
}

void EditIconPopup::onPNG(CCObject* sender) {
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

void EditIconPopup::onPlist(CCObject* sender) {
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

void EditIconPopup::onPreset(CCObject* sender) {
    IconPresetPopup::create(m_iconType, {}, [this](int id, IconInfo* info) {
        MoreIcons::getIconPaths(info, id, m_iconType, m_selectedPNG, m_selectedPlist);
        updateWithSelectedFiles();
    })->show();
}

void EditIconPopup::onSave(CCObject* sender) {
    for (auto& required : m_required) {
        if (!m_frames.contains(required)) return Notify::info("Missing icon{}.", required);
    }

    SaveIconPopup::create(m_parentPopup, this, m_iconType, m_state, m_frames)->show();
}

void EditIconPopup::createControls(const CCPoint& pos, const char* text, std::string_view id, int offset) {
    auto def = offset == 4 || offset == 5 ? 1.0f : 0.0f;
    auto min = offset == 0 || offset == 1 ? -20.0f : offset == 4 || offset == 5 ? -10.0f : 0.0f;
    auto max = offset == 0 || offset == 1 ? 20.0f : offset == 4 || offset == 5 ? 10.0f : 360.0f;
    auto decimals = offset != 2 && offset != 3;

    auto slider = Slider::create(this, menu_selector(EditIconPopup::sliderChanged), 0.75f);
    slider->getThumb()->setTag(offset);
    slider->setPosition(pos - CCPoint { 0.0f, 10.0f });
    slider->setID(fmt::format("{}-slider", id));
    m_mainLayer->addChild(slider);
    m_sliders[offset] = slider;

    auto menu = CCMenu::create();
    menu->setPosition(pos + CCPoint { 0.0f, 10.0f });
    menu->setContentSize({ 150.0f, 30.0f });
    menu->ignoreAnchorPointForPosition(false);
    menu->setID(fmt::format("{}-menu", id));
    m_mainLayer->addChild(menu);

    auto label = CCLabelBMFont::create(text, "goldFont.fnt");
    label->setScale(0.6f);
    label->setID(fmt::format("{}-label", id));
    menu->addChild(label);

    auto input = TextInput::create(60.0f, "Num");
    input->setScale(0.5f);
    input->setCommonFilter(decimals ? CommonFilter::Float : CommonFilter::Uint);
    input->setMaxCharCount(decimals ? 5 : 3);
    input->setDelegate(this, offset);
    input->setID(fmt::format("{}-input", id));
    menu->addChild(input);
    m_inputs[offset] = input;

    auto resetSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    resetSprite->setScale(0.4f);
    auto resetButton = CCMenuItemSpriteExtra::create(resetSprite, this, menu_selector(EditIconPopup::onReset));
    resetButton->setTag(offset);
    resetButton->setID(fmt::format("reset-{}-button", id));
    menu->addChild(resetButton);

    menu->setLayout(RowLayout::create()->setAutoScale(false));
}

void EditIconPopup::sliderChanged(CCObject* sender) {
    auto offset = sender->getTag();
    auto min = offset == 0 || offset == 1 ? -20.0f : offset == 4 || offset == 5 ? -10.0f : 0.0f;
    auto max = offset == 0 || offset == 1 ? 20.0f : offset == 4 || offset == 5 ? 10.0f : 360.0f;
    updateControl(offset, static_cast<SliderThumb*>(sender)->getValue() * (max - min) + min, false, true, true);
    updateTargets();
}

void EditIconPopup::textChanged(CCTextInputNode* input) {
    auto value = jasmine::convert::get<float>(MoreIcons::getText(input));
    updateControl(input->getTag(), value.value_or(0.0f), true, false, value.has_value());
    updateTargets();
}

void EditIconPopup::onReset(CCObject* sender) {
    auto offset = sender->getTag();
    updateControl(offset, offset == 4 || offset == 5 ? 1.0f : 0.0f, true, true, true);
    updateTargets();
}

void EditIconPopup::updateControl(int offset, float value, bool slider, bool input, bool definition) {
    switch (offset) {
        case 0:
            if (!definition) value = m_definition->offsetX;
            m_definition->offsetX = std::clamp(roundf(value * 10.0f) / 10.0f, -20.0f, 20.0f);
            if (slider) m_sliders[0]->setValue((value + 20.0f) / 40.0f);
            if (input) m_inputs[0]->setString(fmt::format("{:.1f}", value));
            break;
        case 1:
            if (!definition) value = m_definition->offsetY;
            m_definition->offsetY = std::clamp(roundf(value * 10.0f) / 10.0f, -20.0f, 20.0f);
            if (slider) m_sliders[1]->setValue((value + 20.0f) / 40.0f);
            if (input) m_inputs[1]->setString(fmt::format("{:.1f}", value));
            break;
        case 2:
            if (!definition) value = m_definition->rotationX;
            m_definition->rotationX = std::clamp(roundf(value), 0.0f, 360.0f);
            if (slider) m_sliders[2]->setValue(value / 360.0f);
            if (input) m_inputs[2]->setString(fmt::format("{:.0f}", value));
            break;
        case 3:
            if (!definition) value = m_definition->rotationY;
            m_definition->rotationY = std::clamp(roundf(value), 0.0f, 360.0f);
            if (slider) m_sliders[3]->setValue(value / 360.0f);
            if (input) m_inputs[3]->setString(fmt::format("{:.0f}", value));
            break;
        case 4:
            if (!definition) value = m_definition->scaleX;
            m_definition->scaleX = std::clamp(roundf(value * 10.0f) / 10.0f, -10.0f, 10.0f);
            if (slider) m_sliders[4]->setValue((value + 10.0f) / 20.0f);
            if (input) m_inputs[4]->setString(fmt::format("{:.1f}", value));
            break;
        case 5:
            if (!definition) value = m_definition->scaleY;
            m_definition->scaleY = std::clamp(roundf(value * 10.0f) / 10.0f, -10.0f, 10.0f);
            if (slider) m_sliders[5]->setValue((value + 10.0f) / 20.0f);
            if (input) m_inputs[5]->setString(fmt::format("{:.1f}", value));
            break;
    }
}

void EditIconPopup::updateControls() {
    updateControl(0, m_definition->offsetX, true, true, true);
    updateControl(1, m_definition->offsetY, true, true, true);
    updateControl(2, m_definition->rotationX, true, true, true);
    updateControl(3, m_definition->rotationY, true, true, true);
    updateControl(4, m_definition->scaleX, true, true, true);
    updateControl(5, m_definition->scaleY, true, true, true);
}

CCMenuItemSpriteExtra* EditIconPopup::addPieceButton(std::string_view suffix, int page, bool required) {
    m_state.definitions.emplace(suffix, FrameDefinition());

    auto pieceFrame = Icons::getFrame("{}{}.png", MoreIcons::getIconName(1, m_iconType), suffix);
    if (pieceFrame) m_frames.emplace(suffix, pieceFrame);
    else pieceFrame = Get::SpriteFrameCache()->spriteFrameByName("GJ_deleteIcon_001.png");
    auto pieceSprite = CCSprite::createWithSpriteFrame(pieceFrame);
    auto pieceButton = CCMenuItemSpriteExtra::create(pieceSprite, this, menu_selector(EditIconPopup::onSelectPiece));
    pieceSprite->setPosition({ 15.0f, 15.0f });
    pieceButton->setContentSize({ 30.0f, 30.0f });
    pieceButton->setUserObject("piece-suffix", ObjWrapper<std::string_view>::create(suffix));
    pieceButton->setID(fmt::format("piece{}", suffix));
    m_pieceMenu->addChild(pieceButton);

    m_pages[page].push_back(pieceButton);
    m_pieces.emplace(suffix, pieceSprite);
    if (required) m_required.push_back(suffix);
    return pieceButton;
}

void EditIconPopup::onSelectPiece(CCObject* sender) {
    auto node = static_cast<CCNode*>(sender);
    auto suffix = static_cast<ObjWrapper<std::string_view>*>(node->getUserObject("piece-suffix"))->getValue();
    if (m_suffix == suffix) return;

    auto it = m_state.definitions.find(suffix);
    if (it == m_state.definitions.end()) return;

    m_suffix = suffix;
    m_definition = &it->second;
    m_targets = m_player->getTargets(suffix);
    updateControls();

    m_selectedPage = m_page;
    m_selectSprite->setVisible(true);
    m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_pieceMenu->convertToWorldSpace(node->getPosition())));
}

CCSprite* EditIconPopup::addColorButton(int type, const char* text, std::string&& id) {
    auto sprite = CCSprite::createWithSpriteFrameName("player_special_01_001.png");
    sprite->setScale(0.85f);
    sprite->setCascadeColorEnabled(true);
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.45f);
    label->setPosition(sprite->getContentSize() / 2.0f);
    sprite->addChild(label);
    auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(EditIconPopup::onColor));
    button->setTag(type);
    button->setID(std::move(id));
    m_colorMenu->addChild(button);
    return sprite;
}

void EditIconPopup::onColor(CCObject* sender) {
    auto type = sender->getTag();
    IconColorPopup::create(type, [this, type](int index) {
        updateColor(type, index);
    })->show();
}

void EditIconPopup::updateColor(int type, int index) {
    switch (type) {
        case 1: {
            m_state.mainColor = index;
            auto mainColor = Constants::getColor(index);
            m_mainColorSprite->setColor(mainColor);
            m_player->setMainColor(mainColor);
            break;
        }
        case 2: {
            m_state.secondaryColor = index;
            auto secondaryColor = Constants::getColor(index);
            m_secondaryColorSprite->setColor(secondaryColor);
            m_player->setSecondaryColor(secondaryColor);
            break;
        }
        case 3: {
            m_state.glowColor = index;
            auto glowColor = Constants::getColor(index);
            m_glowColorSprite->setColor(glowColor);
            m_player->setGlowColor(glowColor);
            break;
        }
    }
}

bool EditIconPopup::updateWithSelectedFiles(bool useSuffix) {
    auto ret = false;
    if (auto imageRes = Load::createFrames(m_selectedPNG, m_selectedPlist, {}, m_iconType, useSuffix ? m_suffix : std::string_view(), false)) {
        auto image = std::move(imageRes).unwrap();
        Load::initTexture(image.texture, image.data.data(), image.width, image.height, false);

        if (useSuffix) {
            if (auto it = image.frames.find(m_suffix); it != image.frames.end()) {
                addFrame(m_suffix, std::move(it->second));
            }
            else {
                eraseFrame(m_suffix);
            }
        }
        else {
            for (auto it = m_frames.begin(); it != m_frames.end();) {
                auto frameIt = image.frames.find(it->first);
                if (frameIt != image.frames.end()) {
                    it->second = std::move(frameIt->second);
                    image.frames.erase(frameIt);
                    ++it;
                }
                else {
                    it = m_frames.erase(it);
                }
            }
            for (auto it = image.frames.begin(); it != image.frames.end(); it = image.frames.erase(it)) {
                m_frames.insert(std::move(*it));
            }
        }

        updatePieces();
        ret = true;
    }
    else if (imageRes.isErr()) Notify::error(imageRes.unwrapErr());

    m_selectedPNG.clear();
    m_selectedPlist.clear();
    return ret;
}

void EditIconPopup::updatePieces() {
    auto crossFrame = Get::SpriteFrameCache()->spriteFrameByName("GJ_deleteIcon_001.png");
    for (auto& [suffix, sprite] : m_pieces) {
        auto it = m_frames.find(suffix);
        auto spriteFrame = it != m_frames.end() ? it->second.data() : nullptr;
        sprite->setDisplayFrame(spriteFrame ? spriteFrame : crossFrame);
        for (auto target : m_player->getTargets(suffix)) {
            target->setVisible(spriteFrame != nullptr);
            if (spriteFrame) target->setDisplayFrame(spriteFrame);
        }
    }

    m_hasChanged = true;
}

void EditIconPopup::goToPage(int page) {
    for (auto sprite : m_pages[m_page]) {
        sprite->setVisible(false);
    }

    auto count = m_pages.size();
    m_page = ((page % count) + count) % count;
    m_selectSprite->setVisible(m_selectedPage == m_page);

    for (auto sprite : m_pages[m_page]) {
        sprite->setVisible(true);
    }

    m_pieceMenu->updateLayout();
}

void EditIconPopup::updateTargets() {
    m_hasChanged = true;

    if (m_definition && !m_targets.empty()) {
        auto [offsetX, offsetY, rotationX, rotationY, scaleX, scaleY] = *m_definition;
        for (auto target : m_targets) {
            target->setPositionX(offsetX);
            target->setPositionY(offsetY);
            target->setRotationX(rotationX);
            target->setRotationY(rotationY);
            target->setScaleX(scaleX);
            target->setScaleY(scaleY);
        }
    }
}

void EditIconPopup::onClose(CCObject* sender) {
    if (!m_hasChanged) return close();

    auto type = m_iconType;
    FLAlertLayer::create(
        this,
        fmt::format("Exit {} Editor", Constants::getSingularUppercase(type)).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} editor</c>?\n<cr>All unsaved changes will be lost!</c>",
            Constants::getSingularLowercase(type)),
        "No",
        "Yes",
        350.0f
    )->show();
}

void EditIconPopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (btn2) close();
}
