#include "EditIconPopup.hpp"
#include "IconColorPopup.hpp"
#include "IconPresetPopup.hpp"
#include "ImageRenderer.hpp"
#include "SaveIconPopup.hpp"
#include "../MoreIconsPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Get.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/string.hpp>
#include <jasmine/convert.hpp>
#include <jasmine/mod.hpp>

using namespace geode::prelude;
using namespace jasmine::mod;

EditIconPopup* EditIconPopup::create(MoreIconsPopup* popup, IconType type) {
    auto ret = new EditIconPopup();
    if (ret->initAnchored(450.0f, 280.0f, popup, type, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCArray* arrayWithObject(CCObject* obj) {
    auto arr = CCArray::create();
    if (obj) arr->addObject(obj);
    return arr;
}

template <class... T>
CCArray* arrayWithObjects(CCArray* parent, T... indices) {
    auto arr = CCArray::create();
    for (auto index : { indices... }) {
        if (auto obj = parent->objectAtIndex(index)) arr->addObject(obj);
    }
    return arr;
}

bool EditIconPopup::setup(MoreIconsPopup* popup, IconType type) {
    setID("EditIconPopup");
    setTitle(fmt::format("{} Editor", MoreIcons::uppercase[(int)type]));
    m_title->setID("edit-icon-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_parentPopup = popup;
    m_pages = CCArray::create();
    m_pieces = CCDictionary::create();
    m_frames = CCDictionary::create();
    m_sliders = CCDictionary::create();
    m_inputs = CCDictionary::create();
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
    piecesBackground->setContentSize({ isRobot ? 260.0f : 330.0f, 45.0f });
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
    m_pieceMenu->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even));
    m_pieceMenu->setID("piece-menu");
    m_mainLayer->addChild(m_pieceMenu);

    m_player = SimplePlayer::create(1);
    m_player->updatePlayerFrame(1, type);
    m_player->m_hasGlowOutline = true;
    m_player->updateColors();

    auto previewNode = CCNode::create();
    previewNode->setPosition({ 51.0f, 205.0f });
    previewNode->setScale(2.0f);
    previewNode->setAnchorPoint({ 0.5f, 0.5f });
    transferPlayerToNode(previewNode, m_player);
    previewNode->setID("preview-node");
    m_mainLayer->addChild(previewNode);

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

        auto isSpider = type == IconType::Spider;
        auto robotSprite = isSpider ? m_player->m_spiderSprite : m_player->m_robotSprite;
        auto firstArray = robotSprite->m_paSprite->m_spriteParts;
        auto secondArray = robotSprite->m_secondArray;
        auto glowArray = robotSprite->m_glowSprite->getChildren();

        addPieceButton("_01_001", 0, arrayWithObjects(firstArray, 3));
        addPieceButton("_01_2_001", 0, arrayWithObjects(secondArray, 3));
        addPieceButton("_01_glow_001", 0, arrayWithObjects(glowArray, 3));
        addPieceButton("_01_extra_001", 0, arrayWithObject(robotSprite->m_extraSprite));
        if (isSpider) {
            addPieceButton("_02_001", 1, arrayWithObjects(firstArray, 0, 1, 5));
            addPieceButton("_02_2_001", 1, arrayWithObjects(secondArray, 0, 1, 5));
            addPieceButton("_02_glow_001", 1, arrayWithObjects(glowArray, 0, 1, 5));
            addPieceButton("_03_001", 2, arrayWithObjects(firstArray, 4));
            addPieceButton("_03_2_001", 2, arrayWithObjects(secondArray, 4));
            addPieceButton("_03_glow_001", 2, arrayWithObjects(glowArray, 4));
            addPieceButton("_04_001", 3, arrayWithObjects(firstArray, 2));
            addPieceButton("_04_2_001", 3, arrayWithObjects(secondArray, 2));
            addPieceButton("_04_glow_001", 3, arrayWithObjects(glowArray, 2));
        }
        else {
            addPieceButton("_02_001", 1, arrayWithObjects(firstArray, 1, 5));
            addPieceButton("_02_2_001", 1, arrayWithObjects(secondArray, 1, 5));
            addPieceButton("_02_glow_001", 1, arrayWithObjects(glowArray, 1, 5));
            addPieceButton("_03_001", 2, arrayWithObjects(firstArray, 0, 4));
            addPieceButton("_03_2_001", 2, arrayWithObjects(secondArray, 0, 4));
            addPieceButton("_03_glow_001", 2, arrayWithObjects(glowArray, 0, 4));
            addPieceButton("_04_001", 3, arrayWithObjects(firstArray, 2, 6));
            addPieceButton("_04_2_001", 3, arrayWithObjects(secondArray, 2, 6));
            addPieceButton("_04_glow_001", 3, arrayWithObjects(glowArray, 2, 6));
        }
    }
    else {
        m_suffix = "_001";

        addPieceButton("_001", 0, arrayWithObject(m_player->m_firstLayer));
        addPieceButton("_2_001", 0, arrayWithObject(m_player->m_secondLayer));
        if (type == IconType::Ufo) addPieceButton("_3_001", 0, arrayWithObject(m_player->m_birdDome));
        addPieceButton("_glow_001", 0, arrayWithObject(m_player->m_outlineSprite));
        addPieceButton("_extra_001", 0, arrayWithObject(m_player->m_detailSprite));
    }

    m_pieceMenu->updateLayout();

    auto colorMenu = CCMenu::create();
    colorMenu->setPosition({ 33.0f, 110.0f });
    colorMenu->setContentSize({ 30.0f, 100.0f });
    colorMenu->setLayout(ColumnLayout::create()->setGap(10.0f)->setAxisReverse(true));
    colorMenu->setID("color-menu");
    m_mainLayer->addChild(colorMenu);

    addColorButton(m_firstColor, colorMenu, "1", "first-color-button");
    addColorButton(m_secondColor, colorMenu, "2", "second-color-button");
    addColorButton(m_thirdColor, colorMenu, "G", "third-color-button");

    colorMenu->updateLayout();

    auto saveMenu = CCMenu::create();
    saveMenu->setPosition({ 69.0f, 110.0f });
    saveMenu->setContentSize({ 30.0f, 100.0f });
    saveMenu->setLayout(ColumnLayout::create()->setGap(10.0f)->setAxisReverse(true));
    saveMenu->setID("save-menu");
    m_mainLayer->addChild(saveMenu);

    auto loadStateFrontSprite = CCSprite::createWithSpriteFrameName("geode.loader/install.png");
    loadStateFrontSprite->setFlipY(true);
    auto loadStateSprite = CircleButtonSprite::create(loadStateFrontSprite, CircleBaseColor::Green, CircleBaseSize::Small);
    loadStateSprite->setScale(0.7f);
    auto loadStateButton = CCMenuItemExt::createSpriteExtra(loadStateSprite, [](auto) {});
    loadStateButton->setID("load-state-button");
    saveMenu->addChild(loadStateButton);

    auto saveStateSprite = CircleButtonSprite::createWithSprite("MI_saveBtn_001.png"_spr, 1.0f, CircleBaseColor::Cyan, CircleBaseSize::Small);
    saveStateSprite->setScale(0.7f);
    auto saveStateButton = CCMenuItemExt::createSpriteExtra(saveStateSprite, [](auto) {});
    saveStateButton->setID("save-state-button");
    saveMenu->addChild(saveStateButton);

    saveMenu->updateLayout();

    m_selectSprite = CCSprite::createWithSpriteFrameName("GJ_select_001.png");
    m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_pieceMenu->convertToWorldSpace(m_pieceMenu->getChildByIndex(0)->getPosition())));
    m_selectSprite->setTag(0);
    m_selectSprite->setID("select-sprite");
    m_mainLayer->addChild(m_selectSprite);

    createControls({ 185.0f, 175.0f }, "Offset X:", "offset-x", -20.0f, 20.0f, 0.0f, true);
    createControls({ 355.0f, 175.0f }, "Offset Y:", "offset-y", -20.0f, 20.0f, 0.0f, true);
    createControls({ 185.0f, 135.0f }, "Rotation X:", "rotation-x", 0.0f, 360.0f, 0.0f, false);
    createControls({ 355.0f, 135.0f }, "Rotation Y:", "rotation-y", 0.0f, 360.0f, 0.0f, false);
    createControls({ 185.0f, 95.0f }, "Scale X:", "scale-x", -10.0f, 10.0f, 1.0f, true);
    createControls({ 355.0f, 95.0f }, "Scale Y:", "scale-y", -10.0f, 10.0f, 1.0f, true);

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
            if (res && res->isErr()) return MoreIcons::notifyFailure("Failed to import PNG file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            if (auto textureRes = ImageRenderer::getTexture(res->unwrap())) {
                auto texture = std::move(textureRes).unwrap();
                m_frames->setObject(CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() }),
                    fmt::format("{}.png", m_suffix));
                updatePieces();
            }
            else if (textureRes.isErr()) return MoreIcons::notifyFailure("Failed to load image: {}", textureRes.unwrapErr());
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
        auto key = fmt::format("{}.png", m_suffix);
        IconPresetPopup::create(m_iconType, key, [this, key](int id, IconInfo* info) {
            auto frame = MoreIcons::getFrame(info
                ? fmt::format("{}{}"_spr, info->name, key)
                : fmt::format("{}{:02}{}", MoreIcons::prefixes[(int)m_iconType], id, key));
            if (frame) m_frames->setObject(frame, key);
            else m_frames->removeObjectForKey(key);
            updatePieces();
        })->show();
    });
    piecePresetButton->setID("piece-preset-button");
    pieceButtonMenu->addChild(piecePresetButton);

    auto pieceClearSprite = ButtonSprite::create("Clear", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    pieceClearSprite->setScale(0.6f);
    auto pieceClearButton = CCMenuItemExt::createSpriteExtra(pieceClearSprite, [this](auto) {
        auto key = fmt::format("{}.png", m_suffix);
        if (m_suffix.ends_with("_extra_001")) {
            m_frames->removeObjectForKey(key);
        }
        else {
            auto emptyFrame = MoreIcons::getFrame("emptyFrame.png"_spr);
            if (!emptyFrame) {
                Autorelease texture = new CCTexture2D();
                texture->initWithData(nullptr, kCCTexture2DPixelFormat_RGBA8888, 0, 0, { 0.0f, 0.0f });
                emptyFrame = CCSpriteFrame::createWithTexture(texture, { 0.0f, 0.0f, 0.0f, 0.0f });
                Get::SpriteFrameCache()->addSpriteFrame(emptyFrame, "emptyFrame.png"_spr);
            }
            m_frames->setObject(emptyFrame, key);
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
            if (res && res->isErr()) return MoreIcons::notifyFailure("Failed to import PNG file: {}", res->unwrapErr());
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
            if (res && res->isErr()) return MoreIcons::notifyFailure("Failed to import Plist file: {}", res->unwrapErr());
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
            for (auto [suffix, sprite] : CCDictionaryExt<std::string_view, CCSprite*>(m_pieces)) {
                auto key = fmt::format("{}.png", suffix);
                auto frame = MoreIcons::getFrame(info
                    ? fmt::format("{}{}"_spr, info->name, key)
                    : fmt::format("{}{:02}{}", MoreIcons::prefixes[(int)m_iconType], id, key));
                if (frame) m_frames->setObject(frame, key);
                else m_frames->removeObjectForKey(key);
            }
            updatePieces();
        })->show();
    });
    presetButton->setID("preset-button");
    iconButtonMenu->addChild(presetButton);

    auto saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f), [this](auto) {
        SaveIconPopup::create(this, m_iconType, m_definitions, m_frames)->show();
    });
    saveButton->setID("save-button");
    iconButtonMenu->addChild(saveButton);

    iconButtonMenu->updateLayout();

    handleTouchPriority(this);

    return true;
}

void EditIconPopup::createControls(const CCPoint& pos, const char* text, std::string_view id, float min, float max, float def, bool decimals) {
    m_settings[id] = def;
    m_definitions[m_suffix][id] = def;

    auto div = max - min;
    auto key = gd::string(id.data(), id.size());

    auto slider = Slider::create(nullptr, nullptr, 0.75f);
    slider->setPosition(pos - CCPoint { 0.0f, 10.0f });
    slider->setValue((def - min) / div);
    slider->setID(fmt::format("{}-slider", id));
    m_mainLayer->addChild(slider);
    m_sliders->setObject(slider, key);

    CCMenuItemExt::assignCallback<SliderThumb>(slider->getThumb(), [this, min, div, decimals, id, key](SliderThumb* sender) {
        auto& value = m_settings[id];
        value = sender->getValue() * div + min;
        value = decimals ? roundf(value * 10.0f) / 10.0f : roundf(value);
        m_definitions[m_suffix][id] = value;
        if (auto input = static_cast<TextInput*>(m_inputs->objectForKey(key))) {
            input->setString(decimals ? fmt::format("{:.1f}", value) : fmt::format("{:.0f}", value));
        }
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

    auto input = TextInput::create(60.0f, "Num");
    input->setScale(0.5f);
    input->setString(decimals ? fmt::format("{:.1f}", def) : fmt::format("{:.0f}", def));
    input->setCommonFilter(decimals ? CommonFilter::Float : CommonFilter::Uint);
    input->setMaxCharCount(decimals ? 5 : 3);
    input->setCallback([this, min, max, div, decimals, id, key](const std::string& str) {
        auto& value = m_settings[id];
        jasmine::convert::toFloat(str, value);
        value = std::clamp(value, min, max);
        m_definitions[m_suffix][id] = value;
        if (auto slider = static_cast<Slider*>(m_sliders->objectForKey(key))) {
            slider->setValue((value - min) / div);
        }
        updateTargets();
    });
    input->setID(fmt::format("{}-input", id));
    menu->addChild(input);
    m_inputs->setObject(input, key);

    auto resetButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.4f, [this, min, div, def, id, key, decimals](auto) {
        auto& value = m_settings[id];
        value = def;
        m_definitions[m_suffix][id] = value;
        if (auto slider = static_cast<Slider*>(m_sliders->objectForKey(key))) {
            slider->setValue((value - min) / div);
        }
        if (auto input = static_cast<TextInput*>(m_inputs->objectForKey(key))) {
            input->setString(decimals ? fmt::format("{:.1f}", value) : fmt::format("{:.0f}", value));
        }
        updateTargets();
    });
    resetButton->setID(fmt::format("reset-{}-button", id));
    menu->addChild(resetButton);

    menu->updateLayout();
}

void EditIconPopup::updateControls(std::string_view id, float minimum, float maximum, float defaultValue, bool decimals) {
    auto& value = m_settings[id];
    value = m_definitions[m_suffix][id].as<float>().unwrapOr(defaultValue);
    auto key = gd::string(id.data(), id.size());
    if (auto slider = static_cast<Slider*>(m_sliders->objectForKey(key))) {
        slider->setValue((value - minimum) / (maximum - minimum));
    }
    if (auto input = static_cast<TextInput*>(m_inputs->objectForKey(key))) {
        input->setString(decimals ? fmt::format("{:.1f}", value) : fmt::format("{:.0f}", value));
    }
}

void EditIconPopup::transferPlayerToNode(CCNode* node, SimplePlayer* player) {
    auto type = m_iconType;
    if (type == IconType::Robot || type == IconType::Spider) {
        auto robotSprite = type == IconType::Spider ? player->m_spiderSprite : player->m_robotSprite;

        auto glowNode = CCNode::create();
        glowNode->setAnchorPoint({ 0.5f, 0.5f });
        node->addChild(glowNode, -1);

        auto headSprite = robotSprite->m_headSprite;
        auto spriteParts = robotSprite->m_paSprite->m_spriteParts;
        auto secondArray = robotSprite->m_secondArray;
        auto glowArray = robotSprite->m_glowSprite->getChildren();
        for (int i = 0; i < spriteParts->count(); i++) {
            auto spritePart = static_cast<CCSpritePart*>(spriteParts->objectAtIndex(i));
            auto position = spritePart->getPosition();
            auto scaleX = spritePart->getScaleX();
            auto scaleY = spritePart->getScaleY();
            auto rotation = spritePart->getRotation();
            auto zOrder = spritePart->getZOrder();

            auto partNode = CCNode::create();
            partNode->setPosition(position);
            partNode->setScaleX(scaleX);
            partNode->setScaleY(scaleY);
            partNode->setRotation(rotation);
            partNode->setAnchorPoint({ 0.5f, 0.5f });
            node->addChild(partNode, zOrder);

            auto secondSprite = static_cast<CCSprite*>(secondArray->objectAtIndex(i));
            secondSprite->removeFromParentAndCleanup(false);
            secondSprite->setPosition({ 0.0f, 0.0f });
            partNode->addChild(secondSprite, -1);

            auto glowParent = CCNode::create();
            glowParent->setPosition(position);
            glowParent->setScaleX(scaleX);
            glowParent->setScaleY(scaleY);
            glowParent->setRotation(rotation);
            glowParent->setAnchorPoint({ 0.5f, 0.5f });
            glowNode->addChild(glowParent, zOrder);

            auto glowSprite = static_cast<CCSprite*>(glowArray->objectAtIndex(i));
            glowSprite->setPosition({ 0.0f, 0.0f });
            spritePart->setScaleX(1.0f);
            spritePart->setScaleY(1.0f);
            spritePart->setRotation(0.0f);
            glowParent->addChild(glowSprite);

            if (spritePart == headSprite) {
                if (auto extraSprite = robotSprite->m_extraSprite) {
                    extraSprite->removeFromParentAndCleanup(false);
                    extraSprite->setPosition({ 0.0f, 0.0f });
                    partNode->addChild(extraSprite, 1);
                }
            }

            spritePart->m_followers->removeAllObjects();
            spritePart->m_hasFollower = false;
            spritePart->setPosition({ 0.0f, 0.0f });
            spritePart->setScaleX(1.0f);
            spritePart->setScaleY(1.0f);
            spritePart->setRotation(0.0f);
            partNode->addChild(spritePart, 0);
        }
    }
    else {
        Ref firstLayer = player->m_firstLayer;
        node->setPosition(node->getPosition() + firstLayer->getPosition());
        node->setScale(node->getScale() * firstLayer->getScale());
        while (firstLayer->getChildrenCount() > 0) {
            auto child = firstLayer->getChildByIndex(0);
            child->removeFromParentAndCleanup(false);
            child->setPosition({ 0.0f, 0.0f });
            node->addChild(child);
        }
        firstLayer->setPosition({ 0.0f, 0.0f });
        firstLayer->setScale(1.0f);
        firstLayer->removeFromParentAndCleanup(false);
        node->addChild(firstLayer, 0);
    }
}

void EditIconPopup::addPieceButton(std::string_view suffix, int page, CCArray* targets) {
    auto pieceFrame = MoreIcons::getFrame(fmt::format("{}01{}.png", MoreIcons::prefixes[(int)m_iconType], suffix));
    if (pieceFrame) m_frames->setObject(pieceFrame, fmt::format("{}.png", suffix));
    else pieceFrame = MoreIcons::getFrame("GJ_deleteIcon_001.png");
    auto pieceSprite = CCSprite::createWithSpriteFrame(pieceFrame);
    auto pieceButton = CCMenuItemExt::createSpriteExtra(pieceSprite, [this, suffix, page](CCMenuItemSpriteExtra* sender) {
        m_suffix = suffix;
        updateControls("offset-x", -20.0f, 20.0f, 0.0f, true);
        updateControls("offset-y", -20.0f, 20.0f, 0.0f, true);
        updateControls("rotation-x", 0.0f, 360.0f, 0.0f, false);
        updateControls("rotation-y", 0.0f, 360.0f, 0.0f, false);
        updateControls("scale-x", -10.0f, 10.0f, 1.0f, true);
        updateControls("scale-y", -10.0f, 10.0f, 1.0f, true);
        m_targets = static_cast<CCArray*>(sender->getUserObject("piece-targets"));
        m_selectSprite->setTag(page);
        m_selectSprite->setVisible(true);
        m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_pieceMenu->convertToWorldSpace(sender->getPosition())));
    });
    pieceButton->setContentSize({ 30.0f, 30.0f });
    pieceSprite->setPosition({ 15.0f, 15.0f });

    if (targets) {
        pieceButton->setUserObject("piece-targets", targets);
        if (m_suffix == suffix) m_targets = targets;
    }
    pieceButton->setID(fmt::format("piece-button{}", suffix));

    if (page == 0) m_pieceMenu->addChild(pieceButton);

    if (m_pages->count() <= page) m_pages->addObject(CCArray::create());
    static_cast<CCArray*>(m_pages->objectAtIndex(page))->addObject(pieceButton);
    m_pieces->setObject(pieceSprite, gd::string(suffix.data(), suffix.size()));
}

void EditIconPopup::addColorButton(int& index, CCMenu* menu, const char* text, std::string id) {
    auto color = Get::GameManager()->colorForIdx(index);
    auto sprite = CCSprite::createWithSpriteFrameName("player_special_01_001.png");
    sprite->setScale(0.85f);
    sprite->setColor(color);
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.45f);
    label->setPosition(sprite->getContentSize() / 2.0f);
    label->setColor(color);
    sprite->addChild(label);
    auto button = CCMenuItemExt::createSpriteExtra(sprite, [this, &index, label, sprite](auto) {
        IconColorPopup::create(index, [this, &index, label, sprite](int newIndex) {
            index = newIndex;
            auto gameManager = Get::GameManager();
            auto color = gameManager->colorForIdx(newIndex);
            sprite->setColor(color);
            label->setColor(color);
            m_player->setColor(m_firstColor == newIndex ? color : gameManager->colorForIdx(m_firstColor));
            m_player->setSecondColor(m_secondColor == newIndex ? color : gameManager->colorForIdx(m_secondColor));
            m_player->enableCustomGlowColor(m_thirdColor == newIndex ? color : gameManager->colorForIdx(m_thirdColor));
            m_player->updateColors();
            m_hasChanged = true;
        })->show();
    });
    button->setID(std::move(id));
    menu->addChild(button);
}

void EditIconPopup::updateWithSelectedFiles() {
    if (auto textureRes = ImageRenderer::getTexture(m_selectedPNG)) {
        auto texture = std::move(textureRes).unwrap();
        if (auto framesRes = Load::createFrames(m_selectedPlist, texture, {}, m_iconType)) {
            auto frames = std::move(framesRes).unwrap();
            m_frames->removeAllObjects();
            for (auto [frameName, frame] : CCDictionaryExt<gd::string, CCSpriteFrame*>(frames)) {
                m_frames->setObject(frame, frameName);
            }
            updatePieces();
        }
        else if (framesRes.isErr()) MoreIcons::notifyFailure("Failed to load frames: {}", framesRes.unwrapErr());
    }
    else if (textureRes.isErr()) MoreIcons::notifyFailure("Failed to load image: {}", textureRes.unwrapErr());

    m_selectedPNG.clear();
    m_selectedPlist.clear();
}

void EditIconPopup::updatePieces() {
    auto crossFrame = Get::SpriteFrameCache()->spriteFrameByName("GJ_deleteIcon_001.png");
    for (auto [suffix, sprite] : CCDictionaryExt<std::string_view, CCSprite*>(m_pieces)) {
        auto spriteFrame = static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("{}.png", suffix)));
        sprite->setDisplayFrame(spriteFrame ? spriteFrame : crossFrame);
    }

    auto type = m_iconType;
    if (type == IconType::Robot || type == IconType::Spider) {
        auto sprite = type == IconType::Spider ? m_player->m_spiderSprite : m_player->m_robotSprite;
        auto spriteParts = sprite->m_paSprite->m_spriteParts;
        for (int i = 0; i < spriteParts->count(); i++) {
            auto spritePart = static_cast<CCSprite*>(spriteParts->objectAtIndex(i));
            auto tag = spritePart->getTag();

            spritePart->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_001.png", tag))));
            if (auto secondSprite = static_cast<CCSprite*>(sprite->m_secondArray->objectAtIndex(i))) {
                secondSprite->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_2_001.png", tag))));
            }

            if (auto glowChild = sprite->m_glowSprite->getChildByIndex<CCSprite>(i)) {
                glowChild->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_glow_001.png", tag))));
            }

            if (spritePart == sprite->m_headSprite) {
                auto extraFrame = static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_extra_001.png", tag)));
                if (extraFrame) {
                    if (sprite->m_extraSprite) sprite->m_extraSprite->setDisplayFrame(extraFrame);
                    else {
                        sprite->m_extraSprite = CCSprite::createWithSpriteFrame(extraFrame);
                        spritePart->addChild(sprite->m_extraSprite, 2);
                    }
                }
                sprite->m_extraSprite->setVisible(extraFrame != nullptr);
            }
        }
    }
    else {
        m_player->m_firstLayer->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_001.png")));
        m_player->m_secondLayer->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_2_001.png")));
        if (type == IconType::Ufo) {
            m_player->m_birdDome->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_3_001.png")));
        }
        m_player->m_outlineSprite->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_glow_001.png")));
        auto extraFrame = static_cast<CCSpriteFrame*>(m_frames->objectForKey("_extra_001.png"));
        m_player->m_detailSprite->setVisible(extraFrame != nullptr);
        if (extraFrame) {
            m_player->m_detailSprite->setDisplayFrame(extraFrame);
        }
    }

    m_hasChanged = true;
}

void EditIconPopup::goToPage(int page) {
    for (auto sprite : static_cast<CCArray*>(m_pages->objectAtIndex(m_page))->asExt<CCNode>()) {
        m_pieceMenu->removeChild(sprite, false);
    }

    auto count = m_pages->count();
    m_page = ((page % count) + count) % count;
    m_selectSprite->setVisible(m_selectSprite->getTag() == m_page);

    for (auto sprite : static_cast<CCArray*>(m_pages->objectAtIndex(m_page))->asExt<CCNode>()) {
        m_pieceMenu->addChild(sprite);
    }

    m_pieceMenu->updateLayout();
}

void EditIconPopup::updateTargets() {
    if (!m_targets) return;

    auto& offsetX = m_settings["offset-x"];
    auto& offsetY = m_settings["offset-y"];
    auto& rotationX = m_settings["rotation-x"];
    auto& rotationY = m_settings["rotation-y"];
    auto& scaleX = m_settings["scale-x"];
    auto& scaleY = m_settings["scale-y"];
    for (int i = 0; i < m_targets->count(); i++) {
        auto target = static_cast<CCSprite*>(m_targets->objectAtIndex(i));
        target->setPositionX(offsetX);
        target->setPositionY(offsetY);
        target->setRotationX(rotationX);
        target->setRotationY(rotationY);
        target->setScaleX(scaleX);
        target->setScaleY(scaleY);
    }

    m_hasChanged = true;
}

void EditIconPopup::onClose(CCObject* sender) {
    if (!m_hasChanged) return Popup::onClose(sender);

    auto type = (int)m_iconType;
    createQuickPopup(
        fmt::format("Exit {} Editor", MoreIcons::uppercase[type]).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} editor</c>?\n<cr>All unsaved changes will be lost!</c>",
            MoreIcons::lowercase[type]),
        "No",
        "Yes",
        [this](auto, bool btn2) {
            if (btn2) Popup::onClose(m_closeBtn);
        }
    );
}

void EditIconPopup::close() {
    m_parentPopup->close();
    Popup::onClose(nullptr);
}
