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
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
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

    auto iconBackground = CCScale9Sprite::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    iconBackground->setPosition({ 51.0f, 205.0f });
    iconBackground->setContentSize({ 80.0f, 80.0f });
    iconBackground->setOpacity(105);
    iconBackground->setID("icon-background");
    m_mainLayer->addChild(iconBackground);

    auto piecesBackground = CCScale9Sprite::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    piecesBackground->setPosition({ 270.0f, 222.0f });
    piecesBackground->setContentSize({ isRobot ? 260.0f / 0.84375f : 330.0f / 0.84375f, 160.0f / 3.0f });
    piecesBackground->setScale(0.84375f);
    piecesBackground->setOpacity(105);
    piecesBackground->setID("pieces-background");
    m_mainLayer->addChild(piecesBackground);

    auto controlBackground = CCScale9Sprite::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    controlBackground->setPosition({ 270.0f, 122.0f });
    controlBackground->setContentSize({ 345.0f, 150.0f });
    controlBackground->setOpacity(50);
    controlBackground->setID("control-background");
    m_mainLayer->addChild(controlBackground);

    m_pieceMenu = CCMenu::create();
    m_pieceMenu->setPosition({ 270.0f, 222.0f });
    m_pieceMenu->setContentSize({ isRobot ? 260.0f : 330.0f, 45.0f });
    m_pieceMenu->ignoreAnchorPointForPosition(false);
    auto layout = RowLayout::create()->setAxisAlignment(AxisAlignment::Even);
    layout->ignoreInvisibleChildren(true);
    m_pieceMenu->setLayout(layout);
    m_pieceMenu->setID("piece-menu");
    m_mainLayer->addChild(m_pieceMenu);

    m_player = SimpleIcon::create(type, MoreIcons::getIconName(1, type));
    m_player->setPosition({ 51.0f, 205.0f });
    m_player->setScale(2.0f);
    m_player->setID("icon-preview");
    m_mainLayer->addChild(m_player);

    if (isRobot) {
        m_suffix = "_01_001";

        auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        prevSprite->setScale(0.8f);
        auto prevButton = CCMenuItemExt::createSpriteExtra(prevSprite, [this](auto) {
            goToPage(m_page - 1);
        });
        prevButton->setPosition({ 120.0f, 222.0f });
        prevButton->setID("prev-button");
        m_buttonMenu->addChild(prevButton);

        auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        nextSprite->setScale(0.8f);
        nextSprite->setFlipX(true);
        auto nextButton = CCMenuItemExt::createSpriteExtra(nextSprite, [this](auto) {
            goToPage(m_page + 1);
        });
        nextButton->setPosition({ 420.0f, 222.0f });
        nextButton->setID("next-button");
        m_buttonMenu->addChild(nextButton);

        addPieceButton("_01_001", 0);
        addPieceButton("_01_2_001", 0);
        addPieceButton("_01_glow_001", 0);
        addPieceButton("_01_extra_001", 0);
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
        m_suffix = "_001";

        addPieceButton("_001", 0);
        addPieceButton("_2_001", 0);
        if (type == IconType::Ufo) addPieceButton("_3_001", 0);
        addPieceButton("_glow_001", 0);
        addPieceButton("_extra_001", 0);
    }

    m_pieceMenu->updateLayout();

    auto colorMenu = CCMenu::create();
    colorMenu->setPosition({ 33.0f, 110.0f });
    colorMenu->setContentSize({ 30.0f, 100.0f });
    colorMenu->setLayout(ColumnLayout::create()->setGap(10.0f)->setAxisReverse(true));
    colorMenu->setID("color-menu");
    m_mainLayer->addChild(colorMenu);

    m_mainColorSprite = addColorButton(m_state.mainColor, colorMenu, "1", "main-color-button");
    m_secondaryColorSprite = addColorButton(m_state.secondaryColor, colorMenu, "2", "secondary-color-button");
    m_glowColorSprite = addColorButton(m_state.glowColor, colorMenu, "G", "glow-color-button");

    colorMenu->updateLayout();

    auto saveMenu = CCMenu::create();
    saveMenu->setPosition({ 69.0f, 110.0f });
    saveMenu->setContentSize({ 30.0f, 100.0f });
    saveMenu->setLayout(ColumnLayout::create()->setGap(10.0f)->setAxisReverse(true));
    saveMenu->setID("save-menu");
    m_mainLayer->addChild(saveMenu);

    auto loadStateSprite = CircleButtonSprite::createWithSprite("MI_loadBtn_001.png"_spr, 1.0f, CircleBaseColor::Green, CircleBaseSize::Small);
    loadStateSprite->setScale(0.7f);
    auto loadStateButton = CCMenuItemExt::createSpriteExtra(loadStateSprite, [this](auto) {
        LoadEditorPopup::create(m_iconType, [this](const std::filesystem::path& directory, std::string_view name) {
            m_selectedPNG = directory / L("icon.png");
            m_selectedPlist = directory / L("icon.plist");
            if (!updateWithSelectedFiles()) return;

            auto stateRes = file::readFromJson<IconEditorState>(directory / L("state.json"));
            if (stateRes.isErr()) return Notify::error("Failed to load {}: {}", name, stateRes.unwrapErr());

            m_state = std::move(stateRes).unwrap();
            updateColors();
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
    });
    loadStateButton->setID("load-state-button");
    saveMenu->addChild(loadStateButton);

    auto saveStateSprite = CircleButtonSprite::createWithSprite("MI_saveBtn_001.png"_spr, 1.0f, CircleBaseColor::Cyan, CircleBaseSize::Small);
    saveStateSprite->setScale(0.7f);
    auto saveStateButton = CCMenuItemExt::createSpriteExtra(saveStateSprite, [this](auto) {
        SaveEditorPopup::create(m_iconType, m_state, m_frames, [this] {
            m_hasChanged = false;
        })->show();
    });
    saveStateButton->setID("save-state-button");
    saveMenu->addChild(saveStateButton);

    saveMenu->updateLayout();

    m_selectSprite = CCSprite::createWithSpriteFrameName("GJ_select_001.png");
    m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_pieceMenu->convertToWorldSpace(m_pieceMenu->getChildByIndex(0)->getPosition())));
    m_selectSprite->setTag(0);
    m_selectSprite->setID("select-sprite");
    m_mainLayer->addChild(m_selectSprite);

    createControls({ 185.0f, 175.0f }, "Offset X:", "offset-x", 0);
    createControls({ 355.0f, 175.0f }, "Offset Y:", "offset-y", 1);
    createControls({ 185.0f, 135.0f }, "Rotation X:", "rotation-x", 2);
    createControls({ 355.0f, 135.0f }, "Rotation Y:", "rotation-y", 3);
    createControls({ 185.0f, 95.0f }, "Scale X:", "scale-x", 4);
    createControls({ 355.0f, 95.0f }, "Scale Y:", "scale-y", 5);

    auto pieceButtonMenu = CCMenu::create();
    pieceButtonMenu->setPosition({ 270.0f, 60.0f });
    pieceButtonMenu->setContentSize({ 330.0f, 20.0f });
    pieceButtonMenu->ignoreAnchorPointForPosition(false);
    pieceButtonMenu->setLayout(RowLayout::create()->setGap(30.0f));
    pieceButtonMenu->setID("piece-button-menu");
    m_mainLayer->addChild(pieceButtonMenu);

    auto pieceImportSprite = ButtonSprite::create("Import", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    pieceImportSprite->setScale(0.6f);
    auto pieceImportButton = CCMenuItemExt::createSpriteExtra(pieceImportSprite, [this](auto) {
        m_listener.bind([this](Task<Result<std::filesystem::path>>::Event* event) {
            auto res = event->getValue();
            if (res && res->isErr()) return Notify::error("Failed to import PNG file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            if (auto textureRes = Load::createTexture(res->unwrap())) {
                m_frames.emplace(m_suffix, frameWithTexture(textureRes.unwrap()));
                updatePieces();
            }
            else if (textureRes.isErr()) return Notify::error(textureRes.unwrapErr());
        });

        m_listener.setFilter(file::pick(file::PickMode::OpenFile, {
            .filters = {{
                .description = "PNG files",
                .files = { "*.png" }
            }}
        }));
    });
    pieceImportButton->setID("piece-import-button");
    pieceButtonMenu->addChild(pieceImportButton);

    auto piecePresetSprite = ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    piecePresetSprite->setScale(0.6f);
    auto piecePresetButton = CCMenuItemExt::createSpriteExtra(piecePresetSprite, [this](auto) {
        IconPresetPopup::create(m_iconType, m_suffix, [this](int id, IconInfo* info) {
            MoreIcons::getIconPaths(info, id, m_iconType, m_selectedPNG, m_selectedPlist);
            updateWithSelectedFiles(true);
        })->show();
    });
    piecePresetButton->setID("piece-preset-button");
    pieceButtonMenu->addChild(piecePresetButton);

    auto pieceClearSprite = ButtonSprite::create("Clear", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    pieceClearSprite->setScale(0.6f);
    auto pieceClearButton = CCMenuItemExt::createSpriteExtra(pieceClearSprite, [this](auto) {
        if (m_suffix.ends_with("_extra_001")) {
            m_frames.erase(m_suffix);
        }
        else {
            auto emptyFrame = Icons::getFrame("emptyFrame.png"_spr);
            if (!emptyFrame) {
                emptyFrame = frameWithTexture(Load::createTexture(nullptr, 0, 0));
                Get::SpriteFrameCache()->addSpriteFrame(emptyFrame, "emptyFrame.png"_spr);
            }
            m_frames.emplace(m_suffix, emptyFrame);
        }
        updatePieces();
    });
    pieceClearButton->setID("piece-clear-button");
    pieceButtonMenu->addChild(pieceClearButton);

    pieceButtonMenu->updateLayout();

    auto iconButtonMenu = CCMenu::create();
    iconButtonMenu->setPosition({ 225.0f, 30.0f });
    iconButtonMenu->setContentSize({ 450.0f, 30.0f });
    iconButtonMenu->ignoreAnchorPointForPosition(false);
    iconButtonMenu->setLayout(RowLayout::create()->setGap(30.0f));
    iconButtonMenu->setID("icon-button-menu");
    m_mainLayer->addChild(iconButtonMenu);

    auto pngSprite = ButtonSprite::create("PNG", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto plistSprite = ButtonSprite::create("Plist", "goldFont.fnt", "GJ_button_05.png", 0.8f);

    auto pngButton = CCMenuItemExt::createSpriteExtra(pngSprite, [this, pngSprite, plistSprite](auto) {
        m_listener.bind([this, pngSprite, plistSprite](Task<Result<std::filesystem::path>>::Event* event) {
            auto res = event->getValue();
            if (res && res->isErr()) return Notify::error("Failed to import PNG file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            m_selectedPNG = res->unwrap();
            if (m_selectedPlist.empty()) {
                pngSprite->updateBGImage("GJ_button_03.png");
                plistSprite->m_BGSprite->runAction(CCRepeat::create(CCSequence::createWithTwoActions(
                    CCEaseIn::create(CCTintTo::create(0.4f, 127, 255, 127), 2.0f),
                    CCEaseOut::create(CCTintTo::create(0.4f, 255, 255, 255), 2.0f)
                ), 2));
                m_hasChanged = true;
            }
            else {
                plistSprite->updateBGImage("GJ_button_05.png");
                updateWithSelectedFiles();
            }
        });

        m_listener.setFilter(file::pick(file::PickMode::OpenFile, {
            .filters = {{
                .description = "PNG files",
                .files = { "*.png" }
            }}
        }));
    });
    pngButton->setID("png-button");
    iconButtonMenu->addChild(pngButton);

    auto plistButton = CCMenuItemExt::createSpriteExtra(plistSprite, [this, pngSprite, plistSprite](auto) {
        m_listener.bind([this, pngSprite, plistSprite](Task<Result<std::filesystem::path>>::Event* event) {
            auto res = event->getValue();
            if (res && res->isErr()) return Notify::error("Failed to import Plist file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            m_selectedPlist = res->unwrap();
            if (m_selectedPNG.empty()) {
                plistSprite->updateBGImage("GJ_button_03.png");
                pngSprite->m_BGSprite->runAction(CCRepeat::create(CCSequence::createWithTwoActions(
                    CCEaseIn::create(CCTintTo::create(0.4f, 127, 255, 127), 2.0f),
                    CCEaseOut::create(CCTintTo::create(0.4f, 255, 255, 255), 2.0f)
                ), 2));
                m_hasChanged = true;
            }
            else {
                pngSprite->updateBGImage("GJ_button_05.png");
                updateWithSelectedFiles();
            }
        });

        m_listener.setFilter(file::pick(file::PickMode::OpenFile, {
            .filters = {{
                .description = "Plist files",
                .files = { "*.plist" }
            }}
        }));
    });
    plistButton->setID("plist-button");
    iconButtonMenu->addChild(plistButton);

    auto presetButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png", 0.8f), [this](auto) {
        IconPresetPopup::create(m_iconType, {}, [this](int id, IconInfo* info) {
            MoreIcons::getIconPaths(info, id, m_iconType, m_selectedPNG, m_selectedPlist);
            updateWithSelectedFiles();
        })->show();
    });
    presetButton->setID("preset-button");
    iconButtonMenu->addChild(presetButton);

    auto saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f), [this](auto) {
        SaveIconPopup::create(m_parentPopup, this, m_iconType, m_state.definitions, m_frames)->show();
    });
    saveButton->setID("save-button");
    iconButtonMenu->addChild(saveButton);

    iconButtonMenu->updateLayout();

    handleTouchPriority(this);

    return true;
}

void EditIconPopup::createControls(const CCPoint& pos, const char* text, std::string_view id, int offset) {
    auto def = *(&m_definition->offsetX + offset);
    auto min = offset == 0 || offset == 1 ? -20.0f : offset == 4 || offset == 5 ? -10.0f : 0.0f;
    auto max = offset == 0 || offset == 1 ? 20.0f : offset == 4 || offset == 5 ? 10.0f : 360.0f;
    auto decimals = offset != 2 && offset != 3;
    auto div = max - min;

    auto slider = Slider::create(nullptr, nullptr, 0.75f);
    auto input = TextInput::create(60.0f, "Num");

    auto sliderValue = (def - min) / div;
    slider->setPosition(pos - CCPoint { 0.0f, 10.0f });
    slider->setValue(sliderValue);
    slider->setID(fmt::format("{}-slider", id));
    m_mainLayer->addChild(slider);
    m_sliders[offset] = slider;

    CCMenuItemExt::assignCallback<SliderThumb>(slider->getThumb(), [this, min, div, decimals, id, input, offset](SliderThumb* sender) {
        auto& value = *(&m_definition->offsetX + offset);
        value = sender->getValue() * div + min;
        value = decimals ? roundf(value * 10.0f) / 10.0f : roundf(value);
        input->setString(decimals ? fmt::format("{:.1f}", value) : fmt::format("{:.0f}", value));
        updateTargets();
    });

    auto menu = CCMenu::create();
    menu->setPosition(pos + CCPoint { 0.0f, 10.0f });
    menu->setContentSize({ 150.0f, 30.0f });
    menu->ignoreAnchorPointForPosition(false);
    menu->setLayout(RowLayout::create()->setAutoScale(false));
    menu->setID(fmt::format("{}-menu", id));
    m_mainLayer->addChild(menu);

    auto label = CCLabelBMFont::create(text, "goldFont.fnt");
    label->setScale(0.6f);
    label->setID(fmt::format("{}-label", id));
    menu->addChild(label);

    auto inputString = decimals ? fmt::format("{:.1f}", def) : fmt::format("{:.0f}", def);
    input->setScale(0.5f);
    input->setString(inputString);
    input->setCommonFilter(decimals ? CommonFilter::Float : CommonFilter::Uint);
    input->setMaxCharCount(decimals ? 5 : 3);
    input->setCallback([this, min, max, div, slider, offset](const std::string& str) {
        auto& value = *(&m_definition->offsetX + offset);
        jasmine::convert::to(str, value);
        value = std::clamp(value, min, max);
        slider->setValue((value - min) / div);
        updateTargets();
    });
    input->setID(fmt::format("{}-input", id));
    menu->addChild(input);
    m_inputs[offset] = input;

    auto resetButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.4f, [
        this, def, slider, input, sliderValue, inputString = std::move(inputString), offset
    ](auto) {
        auto& value = *(&m_definition->offsetX + offset);
        value = def;
        slider->setValue(sliderValue);
        input->setString(inputString);
        updateTargets();
    });
    resetButton->setID(fmt::format("reset-{}-button", id));
    menu->addChild(resetButton);

    menu->updateLayout();
}

void EditIconPopup::updateControls() {
    auto offsetX = m_definition->offsetX;
    m_sliders[0]->setValue((offsetX + 20.0f) / 40.0f);
    m_inputs[0]->setString(fmt::format("{:.1f}", offsetX));

    auto offsetY = m_definition->offsetY;
    m_sliders[1]->setValue((offsetY + 20.0f) / 40.0f);
    m_inputs[1]->setString(fmt::format("{:.1f}", offsetY));

    auto rotationX = m_definition->rotationX;
    m_sliders[2]->setValue(rotationX / 360.0f);
    m_inputs[2]->setString(fmt::format("{:.0f}", rotationX));

    auto rotationY = m_definition->rotationY;
    m_sliders[3]->setValue(rotationY / 360.0f);
    m_inputs[3]->setString(fmt::format("{:.0f}", rotationY));

    auto scaleX = m_definition->scaleX;
    m_sliders[4]->setValue((scaleX + 10.0f) / 20.0f);
    m_inputs[4]->setString(fmt::format("{:.1f}", scaleX));

    auto scaleY = m_definition->scaleY;
    m_sliders[5]->setValue((scaleY + 10.0f) / 20.0f);
    m_inputs[5]->setString(fmt::format("{:.1f}", scaleY));
}

void EditIconPopup::addPieceButton(const std::string& suffix, int page) {
    auto definitionP = &m_state.definitions[suffix];
    auto targetsP = &m_player->getTargets(suffix);
    if (m_suffix == suffix) {
        m_definition = definitionP;
        m_targetsArray = targetsP;
    }

    auto pieceFrame = Icons::getFrame("{}{}.png", MoreIcons::getIconName(1, m_iconType), suffix);
    if (pieceFrame) m_frames.emplace(suffix, pieceFrame);
    else pieceFrame = Get::SpriteFrameCache()->spriteFrameByName("GJ_deleteIcon_001.png");
    auto pieceSprite = CCSprite::createWithSpriteFrame(pieceFrame);
    auto pieceButton = CCMenuItemExt::createSpriteExtra(pieceSprite, [this, suffix, page, definitionP, targetsP](CCMenuItemSpriteExtra* sender) {
        m_suffix = suffix;
        m_definition = definitionP;
        m_targetsArray = targetsP;
        updateControls();
        m_selectSprite->setTag(page);
        m_selectSprite->setVisible(true);
        m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_pieceMenu->convertToWorldSpace(sender->getPosition())));
    });
    pieceSprite->setPosition({ 15.0f, 15.0f });
    pieceButton->setContentSize({ 30.0f, 30.0f });
    pieceButton->setVisible(page == 0);
    pieceButton->setID(fmt::format("piece{}", suffix));
    m_pieceMenu->addChild(pieceButton);

    if (m_pages.size() <= page) m_pages.emplace_back();
    m_pages[page].push_back(pieceButton);
    m_pieces.emplace(suffix, pieceSprite);
}

CCSprite* EditIconPopup::addColorButton(int& index, CCMenu* menu, const char* text, std::string&& id) {
    auto sprite = CCSprite::createWithSpriteFrameName("player_special_01_001.png");
    sprite->setScale(0.85f);
    sprite->setCascadeColorEnabled(true);
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.45f);
    label->setPosition(sprite->getContentSize() / 2.0f);
    sprite->setColor(Get::GameManager()->colorForIdx(index));
    sprite->addChild(label);
    auto button = CCMenuItemExt::createSpriteExtra(sprite, [this, &index](auto) {
        IconColorPopup::create(index, [this, &index](int newIndex) {
            index = newIndex;
            updateColors();
        })->show();
    });
    button->setID(std::move(id));
    menu->addChild(button);
    return sprite;
}

void EditIconPopup::updateColors() {
    auto gameManager = Get::GameManager();

    auto mainColor = gameManager->colorForIdx(m_state.mainColor);
    m_mainColorSprite->setColor(mainColor);

    auto secondaryColor = gameManager->colorForIdx(m_state.secondaryColor);
    m_secondaryColorSprite->setColor(secondaryColor);

    auto glowColor = gameManager->colorForIdx(m_state.glowColor);
    m_glowColorSprite->setColor(glowColor);

    m_player->setColors(mainColor, secondaryColor, glowColor);
}

bool EditIconPopup::updateWithSelectedFiles(bool useSuffix) {
    auto ret = false;
    if (auto imageRes = Load::createFrames(m_selectedPNG, m_selectedPlist, {}, m_iconType, useSuffix ? m_suffix : std::string_view(), false)) {
        auto image = std::move(imageRes).unwrap();
        Load::initTexture(image.texture, image.data.data(), image.width, image.height, false);

        if (useSuffix) {
            if (auto it = image.frames.find(m_suffix); it != image.frames.end()) {
                m_frames[m_suffix] = std::move(it->second);
            }
            else {
                m_frames.erase(m_suffix);
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
            for (auto& pair : image.frames) {
                m_frames.insert(std::move(pair));
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

CCSpriteFrame* EditIconPopup::getFrame(const std::string& suffix) {
    auto it = m_frames.find(suffix);
    if (it != m_frames.end()) return it->second;
    return nullptr;
}

void EditIconPopup::updatePieces() {
    auto crossFrame = Get::SpriteFrameCache()->spriteFrameByName("GJ_deleteIcon_001.png");
    for (auto& [suffix, sprite] : m_pieces) {
        auto spriteFrame = getFrame(suffix);
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
    m_selectSprite->setVisible(m_selectSprite->getTag() == m_page);

    for (auto sprite : m_pages[m_page]) {
        sprite->setVisible(true);
    }

    m_pieceMenu->updateLayout();
}

void EditIconPopup::updateTargets() {
    if (!m_targetsArray || !m_definition) return;

    for (auto target : *m_targetsArray) {
        target->setPositionX(m_definition->offsetX);
        target->setPositionY(m_definition->offsetY);
        target->setRotationX(m_definition->rotationX);
        target->setRotationY(m_definition->rotationY);
        target->setScaleX(m_definition->scaleX);
        target->setScaleY(m_definition->scaleY);
    }

    m_hasChanged = true;
}

void EditIconPopup::onClose(CCObject* sender) {
    if (!m_hasChanged) return Popup::onClose(sender);

    auto type = m_iconType;
    createQuickPopup(
        fmt::format("Exit {} Editor", Constants::getSingularUppercase(type)).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} editor</c>?\n<cr>All unsaved changes will be lost!</c>",
            Constants::getSingularLowercase(type)),
        "No",
        "Yes",
        [this](auto, bool btn2) {
            if (btn2) Popup::onClose(m_closeBtn);
        }
    );
}
