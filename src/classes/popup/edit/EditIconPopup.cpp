#include "EditIconPopup.hpp"
#include "IconPresetPopup.hpp"
#include "SaveIconPopup.hpp"
#include "../MoreIconsPopup.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/string.hpp>
#include <jasmine/convert.hpp>
#include <jasmine/mod.hpp>
#include <texpack.hpp>

using namespace geode::prelude;
using namespace jasmine::mod;

template <typename... T>
void notify(NotificationIcon icon, fmt::format_string<T...> message, T&&... args) {
    Notification::create(fmt::format(message, std::forward<T>(args)...), icon)->show();
}

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
    setTitle(fmt::format("{} Editor", MoreIconsAPI::uppercase[(int)type]));
    m_title->setID("edit-icon-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_parentPopup = popup;
    m_pages = CCArray::create();
    m_definitions = CCDictionary::create();
    m_pieces = CCDictionary::create();
    m_frames = CCDictionary::create();
    m_iconType = type;

    auto isRobot = type == IconType::Robot || type == IconType::Spider;

    auto iconBackground = CCScale9Sprite::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    iconBackground->setPosition({ 55.0f, 205.0f });
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
    previewNode->setPosition({ 55.0f, 205.0f });
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

    m_selectSprite = CCSprite::createWithSpriteFrameName("GJ_select_001.png");
    m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_pieceMenu->convertToWorldSpace(m_pieceMenu->getChildByIndex(0)->getPosition())));
    m_selectSprite->setTag(0);
    m_selectSprite->setID("select-sprite");
    m_mainLayer->addChild(m_selectSprite);

    m_offsetXSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_offsetXSlider->setPosition({ 185.0f, 165.0f });
    m_offsetXSlider->setValue((m_offsetX + 20.0f) / 40.0f);
    m_offsetXSlider->setID("offset-x-slider");
    m_mainLayer->addChild(m_offsetXSlider);

    CCMenuItemExt::assignCallback<SliderThumb>(m_offsetXSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_offsetX = roundf(sender->getValue() * 400.0f - 200.0f) / 10.0f;
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("offsetX{}", m_suffix)))->m_fValue = m_offsetX;
        updateTargets();
        m_offsetXInput->setString(fmt::format("{:.1f}", m_offsetX));
        m_hasChanged = true;
    });

    auto offsetXMenu = CCMenu::create();
    offsetXMenu->setPosition({ 185.0f, 185.0f });
    offsetXMenu->setContentSize({ 150.0f, 30.0f });
    offsetXMenu->ignoreAnchorPointForPosition(false);
    offsetXMenu->setLayout(RowLayout::create()->setAutoScale(false));
    offsetXMenu->setID("offset-x-menu");
    m_mainLayer->addChild(offsetXMenu);

    auto offsetXLabel = CCLabelBMFont::create("Offset X:", "goldFont.fnt");
    offsetXLabel->setScale(0.6f);
    offsetXLabel->setID("offset-x-label");
    offsetXMenu->addChild(offsetXLabel);

    m_offsetXInput = geode::TextInput::create(60.0f, "Num");
    m_offsetXInput->setScale(0.5f);
    m_offsetXInput->setString(fmt::format("{:.1f}", m_offsetX));
    m_offsetXInput->setCommonFilter(CommonFilter::Float);
    m_offsetXInput->setMaxCharCount(5);
    m_offsetXInput->setCallback([this](const std::string& str) {
        jasmine::convert::toFloat(str, m_offsetX);
        m_offsetX = std::clamp(m_offsetX, -20.0f, 20.0f);
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("offsetX{}", m_suffix)))->m_fValue = m_offsetX;
        updateTargets();
        m_offsetXSlider->setValue((m_offsetX + 20.0f) / 40.0f);
        m_hasChanged = true;
    });
    m_offsetXInput->setID("offset-x-input");
    offsetXMenu->addChild(m_offsetXInput);

    auto resetOffsetXButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.4f, [this](auto) {
        m_offsetX = 0.0f;
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("offsetX{}", m_suffix)))->m_fValue = m_offsetX;
        updateTargets();
        m_offsetXSlider->setValue((m_offsetX + 20.0f) / 40.0f);
        m_offsetXInput->setString(fmt::format("{:.1f}", m_offsetX));
        m_hasChanged = true;
    });
    resetOffsetXButton->setID("reset-offset-x-button");
    offsetXMenu->addChild(resetOffsetXButton);

    offsetXMenu->updateLayout();

    m_offsetYSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_offsetYSlider->setPosition({ 355.0f, 165.0f });
    m_offsetYSlider->setValue((m_offsetY + 20.0f) / 40.0f);
    m_offsetYSlider->setID("offset-y-slider");
    m_mainLayer->addChild(m_offsetYSlider);

    CCMenuItemExt::assignCallback<SliderThumb>(m_offsetYSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_offsetY = roundf(sender->getValue() * 400.0f - 200.0f) / 10.0f;
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("offsetY{}", m_suffix)))->m_fValue = m_offsetY;
        updateTargets();
        m_offsetYInput->setString(fmt::format("{:.1f}", m_offsetY));
        m_hasChanged = true;
    });

    auto offsetYMenu = CCMenu::create();
    offsetYMenu->setPosition({ 355.0f, 185.0f });
    offsetYMenu->setContentSize({ 150.0f, 30.0f });
    offsetYMenu->ignoreAnchorPointForPosition(false);
    offsetYMenu->setLayout(RowLayout::create()->setAutoScale(false));
    offsetYMenu->setID("offset-y-menu");
    m_mainLayer->addChild(offsetYMenu);

    auto offsetYLabel = CCLabelBMFont::create("Offset Y:", "goldFont.fnt");
    offsetYLabel->setScale(0.6f);
    offsetYLabel->setID("offset-y-label");
    offsetYMenu->addChild(offsetYLabel);

    m_offsetYInput = geode::TextInput::create(60.0f, "Num");
    m_offsetYInput->setScale(0.5f);
    m_offsetYInput->setString(fmt::format("{:.1f}", m_offsetY));
    m_offsetYInput->setCommonFilter(CommonFilter::Float);
    m_offsetYInput->setMaxCharCount(5);
    m_offsetYInput->setCallback([this](const std::string& str) {
        jasmine::convert::toFloat(str, m_offsetY);
        m_offsetY = std::clamp(m_offsetY, -20.0f, 20.0f);
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("offsetY{}", m_suffix)))->m_fValue = m_offsetY;
        updateTargets();
        m_offsetYSlider->setValue((m_offsetY + 20.0f) / 40.0f);
        m_hasChanged = true;
    });
    m_offsetYInput->setID("offset-y-input");
    offsetYMenu->addChild(m_offsetYInput);

    auto resetOffsetYButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.4f, [this](auto) {
        m_offsetY = 0.0f;
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("offsetY{}", m_suffix)))->m_fValue = m_offsetY;
        updateTargets();
        m_offsetYSlider->setValue((m_offsetY + 20.0f) / 40.0f);
        m_offsetYInput->setString(fmt::format("{:.1f}", m_offsetY));
        m_hasChanged = true;
    });
    resetOffsetYButton->setID("reset-offset-y-button");
    offsetYMenu->addChild(resetOffsetYButton);

    offsetYMenu->updateLayout();

    m_rotationXSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_rotationXSlider->setPosition({ 185.0f, 125.0f });
    m_rotationXSlider->setValue(m_rotationX / 360.0f);
    m_rotationXSlider->setID("rotation-x-slider");
    m_mainLayer->addChild(m_rotationXSlider);

    CCMenuItemExt::assignCallback<SliderThumb>(m_rotationXSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_rotationX = roundf(sender->getValue() * 360.0f);
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("rotationX{}", m_suffix)))->m_fValue = m_rotationX;
        updateTargets();
        m_rotationXInput->setString(fmt::format("{:.0f}", m_rotationX));
        m_hasChanged = true;
    });

    auto rotationXMenu = CCMenu::create();
    rotationXMenu->setPosition({ 185.0f, 145.0f });
    rotationXMenu->setContentSize({ 150.0f, 30.0f });
    rotationXMenu->ignoreAnchorPointForPosition(false);
    rotationXMenu->setLayout(RowLayout::create()->setAutoScale(false));
    rotationXMenu->setID("rotation-x-menu");
    m_mainLayer->addChild(rotationXMenu);

    auto rotationXLabel = CCLabelBMFont::create("Rotation X:", "goldFont.fnt");
    rotationXLabel->setScale(0.6f);
    rotationXLabel->setID("rotation-x-label");
    rotationXMenu->addChild(rotationXLabel);

    m_rotationXInput = geode::TextInput::create(60.0f, "Num");
    m_rotationXInput->setScale(0.5f);
    m_rotationXInput->setString(fmt::format("{:.0f}", m_rotationX));
    m_rotationXInput->setCommonFilter(CommonFilter::Uint);
    m_rotationXInput->setMaxCharCount(3);
    m_rotationXInput->setCallback([this](const std::string& str) {
        jasmine::convert::toFloat(str, m_rotationX);
        m_rotationX = std::clamp(m_rotationX, 0.0f, 360.0f);
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("rotationX{}", m_suffix)))->m_fValue = m_rotationX;
        updateTargets();
        m_rotationXSlider->setValue(m_rotationX / 360.0f);
        m_hasChanged = true;
    });
    m_rotationXInput->setID("rotation-x-input");
    rotationXMenu->addChild(m_rotationXInput);

    auto resetRotationXButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.4f, [this](auto) {
        m_rotationX = 0.0f;
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("rotationX{}", m_suffix)))->m_fValue = m_rotationX;
        updateTargets();
        m_rotationXSlider->setValue(m_rotationX / 360.0f);
        m_rotationXInput->setString(fmt::format("{:.0f}", m_rotationX));
        m_hasChanged = true;
    });
    resetRotationXButton->setID("reset-rotation-x-button");
    rotationXMenu->addChild(resetRotationXButton);

    rotationXMenu->updateLayout();

    m_rotationYSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_rotationYSlider->setPosition({ 355.0f, 125.0f });
    m_rotationYSlider->setValue(m_rotationY / 360.0f);
    m_rotationYSlider->setID("rotation-y-slider");
    m_mainLayer->addChild(m_rotationYSlider);

    CCMenuItemExt::assignCallback<SliderThumb>(m_rotationYSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_rotationY = roundf(sender->getValue() * 360.0f);
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("rotationY{}", m_suffix)))->m_fValue = m_rotationY;
        updateTargets();
        m_rotationYInput->setString(fmt::format("{:.0f}", m_rotationY));
        m_hasChanged = true;
    });

    auto rotationYMenu = CCMenu::create();
    rotationYMenu->setPosition({ 355.0f, 145.0f });
    rotationYMenu->setContentSize({ 150.0f, 30.0f });
    rotationYMenu->ignoreAnchorPointForPosition(false);
    rotationYMenu->setLayout(RowLayout::create()->setAutoScale(false));
    rotationYMenu->setID("rotation-y-menu");
    m_mainLayer->addChild(rotationYMenu);

    auto rotationYLabel = CCLabelBMFont::create("Rotation Y:", "goldFont.fnt");
    rotationYLabel->setScale(0.6f);
    rotationYLabel->setID("rotation-y-label");
    rotationYMenu->addChild(rotationYLabel);

    m_rotationYInput = geode::TextInput::create(60.0f, "Num");
    m_rotationYInput->setScale(0.5f);
    m_rotationYInput->setString(fmt::format("{:.0f}", m_rotationY));
    m_rotationYInput->setCommonFilter(CommonFilter::Uint);
    m_rotationYInput->setMaxCharCount(3);
    m_rotationYInput->setCallback([this](const std::string& str) {
        jasmine::convert::toFloat(str, m_rotationY);
        m_rotationY = std::clamp(m_rotationY, 0.0f, 360.0f);
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("rotationY{}", m_suffix)))->m_fValue = m_rotationY;
        updateTargets();
        m_rotationYSlider->setValue(m_rotationY / 360.0f);
        m_hasChanged = true;
    });
    m_rotationYInput->setID("rotation-y-input");
    rotationYMenu->addChild(m_rotationYInput);

    auto resetRotationYButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.4f, [this](auto) {
        m_rotationY = 0.0f;
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("rotationY{}", m_suffix)))->m_fValue = m_rotationY;
        updateTargets();
        m_rotationYSlider->setValue(m_rotationY / 360.0f);
        m_rotationYInput->setString(fmt::format("{:.0f}", m_rotationY));
        m_hasChanged = true;
    });
    resetRotationYButton->setID("reset-rotation-y-button");
    rotationYMenu->addChild(resetRotationYButton);

    rotationYMenu->updateLayout();

    m_scaleXSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_scaleXSlider->setPosition({ 185.0f, 85.0f });
    m_scaleXSlider->setValue((m_scaleX + 10.0f) / 20.0f);
    m_scaleXSlider->setID("scale-x-slider");
    m_mainLayer->addChild(m_scaleXSlider);

    CCMenuItemExt::assignCallback<SliderThumb>(m_scaleXSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_scaleX = roundf(sender->getValue() * 200.0f - 100.0f) / 10.0f;
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("scaleX{}", m_suffix)))->m_fValue = m_scaleX;
        updateTargets();
        m_scaleXInput->setString(fmt::format("{:.1f}", m_scaleX));
        m_hasChanged = true;
    });

    auto scaleXMenu = CCMenu::create();
    scaleXMenu->setPosition({ 185.0f, 105.0f });
    scaleXMenu->setContentSize({ 150.0f, 30.0f });
    scaleXMenu->ignoreAnchorPointForPosition(false);
    scaleXMenu->setLayout(RowLayout::create()->setAutoScale(false));
    scaleXMenu->setID("scale-x-menu");
    m_mainLayer->addChild(scaleXMenu);

    auto scaleXLabel = CCLabelBMFont::create("Scale X:", "goldFont.fnt");
    scaleXLabel->setScale(0.6f);
    scaleXLabel->setID("scale-x-label");
    scaleXMenu->addChild(scaleXLabel);

    m_scaleXInput = geode::TextInput::create(60.0f, "Num");
    m_scaleXInput->setScale(0.5f);
    m_scaleXInput->setString(fmt::format("{:.1f}", m_scaleX));
    m_scaleXInput->setCommonFilter(CommonFilter::Float);
    m_scaleXInput->setMaxCharCount(5);
    m_scaleXInput->setCallback([this](const std::string& str) {
        jasmine::convert::toFloat(str, m_scaleX);
        m_scaleX = std::clamp(m_scaleX, -10.0f, 10.0f);
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("scaleX{}", m_suffix)))->m_fValue = m_scaleX;
        updateTargets();
        m_scaleXSlider->setValue((m_scaleX + 10.0f) / 20.0f);
        m_hasChanged = true;
    });
    m_scaleXInput->setID("scale-x-input");
    scaleXMenu->addChild(m_scaleXInput);

    auto resetScaleXButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.4f, [this](auto) {
        m_scaleX = 1.0f;
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("scaleX{}", m_suffix)))->m_fValue = m_scaleX;
        updateTargets();
        m_scaleXSlider->setValue((m_scaleX + 10.0f) / 20.0f);
        m_scaleXInput->setString(fmt::format("{:.1f}", m_scaleX));
        m_hasChanged = true;
    });
    resetScaleXButton->setID("reset-scale-x-button");
    scaleXMenu->addChild(resetScaleXButton);

    scaleXMenu->updateLayout();

    m_scaleYSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_scaleYSlider->setPosition({ 355.0f, 85.0f });
    m_scaleYSlider->setValue((m_scaleY + 10.0f) / 20.0f);
    m_scaleYSlider->setID("scale-y-slider");
    m_mainLayer->addChild(m_scaleYSlider);

    CCMenuItemExt::assignCallback<SliderThumb>(m_scaleYSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_scaleY = roundf(sender->getValue() * 200.0f - 100.0f) / 10.0f;
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("scaleY{}", m_suffix)))->m_fValue = m_scaleY;
        updateTargets();
        m_scaleYInput->setString(fmt::format("{:.1f}", m_scaleY));
        m_hasChanged = true;
    });

    auto scaleYMenu = CCMenu::create();
    scaleYMenu->setPosition({ 355.0f, 105.0f });
    scaleYMenu->setContentSize({ 150.0f, 30.0f });
    scaleYMenu->ignoreAnchorPointForPosition(false);
    scaleYMenu->setLayout(RowLayout::create()->setAutoScale(false));
    scaleYMenu->setID("scale-y-menu");
    m_mainLayer->addChild(scaleYMenu);

    auto scaleYLabel = CCLabelBMFont::create("Scale Y:", "goldFont.fnt");
    scaleYLabel->setScale(0.6f);
    scaleYLabel->setID("scale-y-label");
    scaleYMenu->addChild(scaleYLabel);

    m_scaleYInput = geode::TextInput::create(60.0f, "Num");
    m_scaleYInput->setScale(0.5f);
    m_scaleYInput->setString(fmt::format("{:.1f}", m_scaleY));
    m_scaleYInput->setCommonFilter(CommonFilter::Float);
    m_scaleYInput->setMaxCharCount(5);
    m_scaleYInput->setCallback([this](const std::string& str) {
        jasmine::convert::toFloat(str, m_scaleY);
        m_scaleY = std::clamp(m_scaleY, -10.0f, 10.0f);
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("scaleY{}", m_suffix)))->m_fValue = m_scaleY;
        updateTargets();
        m_scaleYSlider->setValue((m_scaleY + 10.0f) / 20.0f);
        m_hasChanged = true;
    });
    m_scaleYInput->setID("scale-y-input");
    scaleYMenu->addChild(m_scaleYInput);

    auto resetScaleYButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 0.4f, [this](auto) {
        m_scaleY = 1.0f;
        static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("scaleY{}", m_suffix)))->m_fValue = m_scaleY;
        updateTargets();
        m_scaleYSlider->setValue((m_scaleY + 10.0f) / 20.0f);
        m_scaleYInput->setString(fmt::format("{:.1f}", m_scaleY));
        m_hasChanged = true;
    });
    resetScaleYButton->setID("reset-scale-y-button");
    scaleYMenu->addChild(resetScaleYButton);

    scaleYMenu->updateLayout();

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
            if (res && res->isErr()) return notify(NotificationIcon::Error, "Failed to import PNG file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            auto imageRes = texpack::fromPNG(res->unwrap());
            if (imageRes.isErr()) return notify(NotificationIcon::Error, "Failed to load image: {}", imageRes.unwrapErr());

            auto image = std::move(imageRes).unwrap();
            Autorelease texture = new CCTexture2D();
            texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
                (float)image.width,
                (float)image.height
            });

            m_frames->setObject(CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() }),
                fmt::format("{}.png", m_suffix));
            updatePieces();
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
            auto frame = MoreIconsAPI::getFrame(info
                ? fmt::format("{}{}"_spr, info->name, key)
                : fmt::format("{}{:02}{}", MoreIconsAPI::prefixes[(int)m_iconType], id, key));
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
        auto emptyFrame = MoreIconsAPI::getFrame("emptyFrame.png"_spr);
        if (!emptyFrame) {
            Autorelease texture = new CCTexture2D();
            auto factor = MoreIconsAPI::getDirector()->getContentScaleFactor();
            std::vector<uint8_t> data(4.0f * factor * factor, 0);
            texture->initWithData(data.data(), kCCTexture2DPixelFormat_RGBA8888, factor, factor, { factor, factor });
            emptyFrame = CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() });
            MoreIconsAPI::getSpriteFrameCache()->addSpriteFrame(emptyFrame, "emptyFrame.png"_spr);
        }
        m_frames->setObject(emptyFrame, fmt::format("{}.png", m_suffix));
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

    auto pngButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("PNG", "goldFont.fnt", "GJ_button_05.png", 0.8f), [this](auto) {
        m_listener.bind([this](Task<Result<std::filesystem::path>>::Event* event) {
            auto res = event->getValue();
            if (res && res->isErr()) return notify(NotificationIcon::Error, "Failed to import PNG file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            m_selectedPNG = res->unwrap();
            if (!m_selectedPlist.empty()) updateWithSelectedFiles();
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

    auto plistButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Plist", "goldFont.fnt", "GJ_button_05.png", 0.8f), [this](auto) {
        m_listener.bind([this](Task<Result<std::filesystem::path>>::Event* event) {
            auto res = event->getValue();
            if (res && res->isErr()) return notify(NotificationIcon::Error, "Failed to import Plist file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            m_selectedPlist = res->unwrap();
            if (!m_selectedPNG.empty()) updateWithSelectedFiles();
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
                auto frame = MoreIconsAPI::getFrame(info
                    ? fmt::format("{}{}"_spr, info->name, key)
                    : fmt::format("{}{:02}{}", MoreIconsAPI::prefixes[(int)m_iconType], id, key));
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

            spritePart->removeFromParentAndCleanup(false);
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
    m_definitions->setObject(CCFloat::create(0.0f), fmt::format("offsetX{}", suffix));
    m_definitions->setObject(CCFloat::create(0.0f), fmt::format("offsetY{}", suffix));
    m_definitions->setObject(CCFloat::create(0.0f), fmt::format("rotationX{}", suffix));
    m_definitions->setObject(CCFloat::create(0.0f), fmt::format("rotationY{}", suffix));
    m_definitions->setObject(CCFloat::create(1.0f), fmt::format("scaleX{}", suffix));
    m_definitions->setObject(CCFloat::create(1.0f), fmt::format("scaleY{}", suffix));

    auto pieceFrame = MoreIconsAPI::getFrame(fmt::format("{}01{}.png", MoreIconsAPI::prefixes[(int)m_iconType], suffix));
    if (pieceFrame) m_frames->setObject(pieceFrame, fmt::format("{}.png", suffix));
    else pieceFrame = MoreIconsAPI::getFrame("GJ_deleteIcon_001.png");
    auto pieceSprite = CCSprite::createWithSpriteFrame(pieceFrame);
    auto pieceButton = CCMenuItemExt::createSpriteExtra(pieceSprite, [this, suffix, page](CCMenuItemSpriteExtra* sender) {
        m_suffix = suffix;
        m_offsetX = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("offsetX{}", suffix)))->getValue();
        m_offsetXSlider->setValue((m_offsetX + 20.0f) / 40.0f);
        m_offsetXInput->setString(fmt::format("{:.1f}", m_offsetX));
        m_offsetY = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("offsetY{}", suffix)))->getValue();
        m_offsetYSlider->setValue((m_offsetY + 20.0f) / 40.0f);
        m_offsetYInput->setString(fmt::format("{:.1f}", m_offsetY));
        m_rotationX = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("rotationX{}", suffix)))->getValue();
        m_rotationXSlider->setValue(m_rotationX / 360.0f);
        m_rotationXInput->setString(fmt::format("{:.0f}", m_rotationX));
        m_rotationY = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("rotationY{}", suffix)))->getValue();
        m_rotationYSlider->setValue(m_rotationY / 360.0f);
        m_rotationYInput->setString(fmt::format("{:.0f}", m_rotationY));
        m_scaleX = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("scaleX{}", suffix)))->getValue();
        m_scaleXSlider->setValue((m_scaleX + 10.0f) / 20.0f);
        m_scaleXInput->setString(fmt::format("{:.1f}", m_scaleX));
        m_scaleY = static_cast<CCFloat*>(m_definitions->objectForKey(fmt::format("scaleY{}", suffix)))->getValue();
        m_scaleYSlider->setValue((m_scaleY + 10.0f) / 20.0f);
        m_scaleYInput->setString(fmt::format("{:.1f}", m_scaleY));
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

void EditIconPopup::updateWithSelectedFiles() {
    auto imageRes = texpack::fromPNG(m_selectedPNG);
    if (imageRes.isErr()) {
        notify(NotificationIcon::Error, "Failed to load image: {}", imageRes.unwrapErr());
        m_selectedPNG.clear();
        m_selectedPlist.clear();
        return;
    }

    auto image = std::move(imageRes).unwrap();

    Autorelease texture = new CCTexture2D();
    texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
        (float)image.width,
        (float)image.height
    });

    auto framesRes = MoreIconsAPI::createFrames(string::pathToString(m_selectedPlist), texture, "", m_iconType);
    if (framesRes.isErr()) {
        notify(NotificationIcon::Error, "Failed to load frames: {}", framesRes.unwrapErr());
        m_selectedPNG.clear();
        m_selectedPlist.clear();
        return;
    }

    auto frames = std::move(framesRes).unwrap();
    m_frames->removeAllObjects();
    for (auto [frameName, frame] : CCDictionaryExt<gd::string, CCSpriteFrame*>(frames)) {
        m_frames->setObject(frame, frameName);
    }
    updatePieces();
}

void EditIconPopup::updatePieces() {
    auto crossFrame = MoreIconsAPI::getSpriteFrameCache()->spriteFrameByName("GJ_deleteIcon_001.png");
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
    for (auto sprite : CCArrayExt<CCSprite*>(static_cast<CCArray*>(m_pages->objectAtIndex(m_page)))) {
        m_pieceMenu->removeChild(sprite, false);
    }

    auto count = m_pages->count();
    m_page = ((page % count) + count) % count;
    m_selectSprite->setVisible(m_selectSprite->getTag() == m_page);

    for (auto sprite : CCArrayExt<CCSprite*>(static_cast<CCArray*>(m_pages->objectAtIndex(m_page)))) {
        m_pieceMenu->addChild(sprite);
    }

    m_pieceMenu->updateLayout();
}

void EditIconPopup::updateTargets() {
    if (!m_targets) return;

    for (int i = 0; i < m_targets->count(); i++) {
        auto target = static_cast<CCSprite*>(m_targets->objectAtIndex(i));
        target->setPositionX(m_offsetX);
        target->setPositionY(m_offsetY);
        target->setRotationX(m_rotationX);
        target->setRotationY(m_rotationY);
        target->setScaleX(m_scaleX);
        target->setScaleY(m_scaleY);
    }
}

void EditIconPopup::onClose(CCObject* sender) {
    if (!m_hasChanged) return Popup::onClose(sender);

    auto type = (int)m_iconType;
    createQuickPopup(
        fmt::format("Exit {} Editor", MoreIconsAPI::uppercase[type]).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} editor</c>?\n<cr>All unsaved changes will be lost!</c>",
            MoreIconsAPI::lowercase[type]),
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
