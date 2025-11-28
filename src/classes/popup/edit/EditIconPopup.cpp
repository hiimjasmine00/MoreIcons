#include "EditIconPopup.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/Slider.hpp>

using namespace geode::prelude;

EditIconPopup* EditIconPopup::create(MoreIconsPopup* parent, IconType type) {
    auto ret = new EditIconPopup();
    if (ret->initAnchored(450.0f, 280.0f, parent, type, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool EditIconPopup::setup(MoreIconsPopup* parent, IconType type) {
    setID("EditIconPopup");
    setTitle(fmt::format("{} Editor", MoreIconsAPI::uppercase[(int)type]));
    m_title->setID("edit-icon-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

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
    piecesBackground->setContentSize({ isRobot ? 260.0f : 330.f, 45.0f });
    piecesBackground->setOpacity(105);
    piecesBackground->setID("pieces-background");
    m_mainLayer->addChild(piecesBackground);

    if (isRobot) {
        auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        prevSprite->setScale(0.8f);
        auto prevButton = CCMenuItemExt::createSpriteExtra(prevSprite, [](auto) {});
        prevButton->setPosition({ 120.0f, 222.0f });
        prevButton->setID("prev-button");
        m_mainLayer->addChild(prevButton);

        auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        nextSprite->setScale(0.8f);
        nextSprite->setFlipX(true);
        auto nextButton = CCMenuItemExt::createSpriteExtra(nextSprite, [](auto) {});
        nextButton->setPosition({ 420.0f, 222.0f });
        nextButton->setID("next-button");
        m_mainLayer->addChild(nextButton);
    }

    auto offsetXSlider = Slider::create(nullptr, nullptr, 0.75f);
    offsetXSlider->setPosition({ 185.0f, 165.0f });
    offsetXSlider->setValue((m_offsetX + 20.0f) / 40.0f);
    offsetXSlider->setID("offset-x-slider");
    m_mainLayer->addChild(offsetXSlider);

    auto offsetXLabel = CCLabelBMFont::create(fmt::format("Offset X: {:.1f}", m_offsetX).c_str(), "goldFont.fnt");
    offsetXLabel->setPosition({ 185.0f, 185.0f });
    offsetXLabel->setScale(0.6f);
    offsetXLabel->setID("offset-x-label");
    m_mainLayer->addChild(offsetXLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(offsetXSlider->m_touchLogic->m_thumb, [this, offsetXLabel](SliderThumb* sender) {
        m_offsetX = roundf(sender->getValue() * 400.0f - 200.0f) / 10.0f;
        offsetXLabel->setString(fmt::format("Offset X: {:.1f}", m_offsetX).c_str());
        m_hasChanged = true;
    });

    auto offsetYSlider = Slider::create(nullptr, nullptr, 0.75f);
    offsetYSlider->setPosition({ 355.0f, 165.0f });
    offsetYSlider->setValue((m_offsetY + 20.0f) / 40.0f);
    offsetYSlider->setID("offset-y-slider");
    m_mainLayer->addChild(offsetYSlider);

    auto offsetYLabel = CCLabelBMFont::create(fmt::format("Offset Y: {:.1f}", m_offsetY).c_str(), "goldFont.fnt");
    offsetYLabel->setPosition({ 355.0f, 185.0f });
    offsetYLabel->setScale(0.6f);
    offsetYLabel->setID("offset-y-label");
    m_mainLayer->addChild(offsetYLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(offsetYSlider->m_touchLogic->m_thumb, [this, offsetYLabel](SliderThumb* sender) {
        m_offsetY = roundf(sender->getValue() * 400.0f - 200.0f) / 10.0f;
        offsetYLabel->setString(fmt::format("Offset Y: {:.1f}", m_offsetY).c_str());
        m_hasChanged = true;
    });

    auto rotationXSlider = Slider::create(nullptr, nullptr, 0.75f);
    rotationXSlider->setPosition({ 185.0f, 125.0f });
    rotationXSlider->setValue(m_rotationX / 360.0f);
    rotationXSlider->setID("rotation-x-slider");
    m_mainLayer->addChild(rotationXSlider);

    auto rotationXLabel = CCLabelBMFont::create(fmt::format("Rotation X: {:.0f}", m_rotationX).c_str(), "goldFont.fnt");
    rotationXLabel->setPosition({ 185.0f, 145.0f });
    rotationXLabel->setScale(0.6f);
    rotationXLabel->setID("rotation-x-label");
    m_mainLayer->addChild(rotationXLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(rotationXSlider->m_touchLogic->m_thumb, [this, rotationXLabel](SliderThumb* sender) {
        m_rotationX = roundf(sender->getValue() * 360.0f);
        rotationXLabel->setString(fmt::format("Rotation X: {:.0f}", m_rotationX).c_str());
        m_hasChanged = true;
    });

    auto rotationYSlider = Slider::create(nullptr, nullptr, 0.75f);
    rotationYSlider->setPosition({ 355.0f, 125.0f });
    rotationYSlider->setValue(m_rotationY / 360.0f);
    rotationYSlider->setID("rotation-y-slider");
    m_mainLayer->addChild(rotationYSlider);

    auto rotationYLabel = CCLabelBMFont::create(fmt::format("Rotation Y: {:.0f}", m_rotationY).c_str(), "goldFont.fnt");
    rotationYLabel->setPosition({ 355.0f, 145.0f });
    rotationYLabel->setScale(0.6f);
    rotationYLabel->setID("rotation-y-label");
    m_mainLayer->addChild(rotationYLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(rotationYSlider->m_touchLogic->m_thumb, [this, rotationYLabel](SliderThumb* sender) {
        m_rotationY = roundf(sender->getValue() * 360.0f);
        rotationYLabel->setString(fmt::format("Rotation Y: {:.0f}", m_rotationY).c_str());
        m_hasChanged = true;
    });

    auto scaleXSlider = Slider::create(nullptr, nullptr, 0.75f);
    scaleXSlider->setPosition({ 185.0f, 85.0f });
    scaleXSlider->setValue((m_scaleX + 10.0f) / 20.0f);
    scaleXSlider->setID("scale-x-slider");
    m_mainLayer->addChild(scaleXSlider);

    auto scaleXLabel = CCLabelBMFont::create(fmt::format("Scale X: {:.1f}", m_scaleX).c_str(), "goldFont.fnt");
    scaleXLabel->setPosition({ 185.0f, 105.0f });
    scaleXLabel->setScale(0.6f);
    scaleXLabel->setID("scale-x-label");
    m_mainLayer->addChild(scaleXLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(scaleXSlider->m_touchLogic->m_thumb, [this, scaleXLabel](SliderThumb* sender) {
        m_scaleX = roundf(sender->getValue() * 200.0f - 100.0f) / 10.0f;
        scaleXLabel->setString(fmt::format("Scale X: {:.1f}", m_scaleX).c_str());
        m_hasChanged = true;
    });

    auto scaleYSlider = Slider::create(nullptr, nullptr, 0.75f);
    scaleYSlider->setPosition({ 355.0f, 85.0f });
    scaleYSlider->setValue((m_scaleY + 10.0f) / 20.0f);
    scaleYSlider->setID("scale-y-slider");
    m_mainLayer->addChild(scaleYSlider);

    auto scaleYLabel = CCLabelBMFont::create(fmt::format("Scale Y: {:.1f}", m_scaleY).c_str(), "goldFont.fnt");
    scaleYLabel->setPosition({ 355.0f, 105.0f });
    scaleYLabel->setScale(0.6f);
    scaleYLabel->setID("scale-y-label");
    m_mainLayer->addChild(scaleYLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(scaleYSlider->m_touchLogic->m_thumb, [this, scaleYLabel](SliderThumb* sender) {
        m_scaleY = roundf(sender->getValue() * 200.0f - 100.0f) / 10.0f;
        scaleYLabel->setString(fmt::format("Scale Y: {:.1f}", m_scaleY).c_str());
        m_hasChanged = true;
    });

    handleTouchPriority(this);

    return true;
}

void EditIconPopup::onClose(cocos2d::CCObject* sender) {
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
