#include "SpecialSettingsPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/ui/TextInput.hpp>
#include <IconInfo.hpp>

using namespace geode::prelude;

SpecialSettingsPopup* SpecialSettingsPopup::create(IconInfo* info) {
    auto ret = new SpecialSettingsPopup();
    if (ret->initAnchored(400.0f, 150.0f, info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SpecialSettingsPopup::setup(IconInfo* info) {
    setID("SpecialSettingsPopup");
    setTitle("Trail Settings", "goldFont.fnt", 0.7f, 15.0f);
    m_title->setID("trail-settings-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_trailInfo = info->trailInfo;

    auto fadeTimeSlider = Slider::create(nullptr, nullptr, 0.8f);
    fadeTimeSlider->setPosition({ 100.0f, 87.5f });
    fadeTimeSlider->setValue(roundf(m_trailInfo.fade * 50.0f) / 100.0f);
    fadeTimeSlider->setID("fade-time-slider");
    m_mainLayer->addChild(fadeTimeSlider);

    auto fadeTimeLabel = CCLabelBMFont::create("Fade Time:", "goldFont.fnt");
    fadeTimeLabel->setPosition({ 80.0f, 110.0f });
    fadeTimeLabel->setScale(0.7f);
    fadeTimeLabel->setID("fade-time-label");
    m_mainLayer->addChild(fadeTimeLabel);

    auto fadeTimeInput = TextInput::create(60.0f, "Num");
    fadeTimeInput->setPosition({ 150.0f, 110.0f });
    fadeTimeInput->setScale(0.6f);
    fadeTimeInput->setString(fmt::format("{:.2f}", m_trailInfo.fade));
    fadeTimeInput->setFilter(".0123456789");
    fadeTimeInput->setMaxCharCount(4);
    fadeTimeInput->setCallback([this, fadeTimeSlider](const std::string& str) {
        #ifdef __cpp_lib_to_chars
        std::from_chars(str.data(), str.data() + str.size(), m_trailInfo.fade);
        #else
        GEODE_UNWRAP_INTO_IF_OK(m_trailInfo.fade, numFromString<float>(str));
        #endif
        m_trailInfo.fade = std::clamp(m_trailInfo.fade * 100.0f, 0.0f, 200.0f) / 100.0f;
        fadeTimeSlider->setValue(m_trailInfo.fade / 2.0f);
    });
    fadeTimeInput->setID("fade-time-input");
    m_mainLayer->addChild(fadeTimeInput);

    CCMenuItemExt::assignCallback<SliderThumb>(fadeTimeSlider->m_touchLogic->m_thumb, [this, fadeTimeInput](SliderThumb* sender) {
        m_trailInfo.fade = roundf(sender->getValue() * 200.0f) / 100.0f;
        fadeTimeInput->setString(fmt::format("{:.2f}", m_trailInfo.fade));
    });

    auto strokeWidthSlider = Slider::create(nullptr, nullptr, 0.8f);
    strokeWidthSlider->setPosition({ 300.0f, 87.5f });
    strokeWidthSlider->setValue(roundf(m_trailInfo.stroke / 2.0f) / 10.0f);
    strokeWidthSlider->m_touchLogic->m_thumb->setTag(2);
    strokeWidthSlider->setID("stroke-width-slider");
    m_mainLayer->addChild(strokeWidthSlider);

    auto strokeWidthLabel = CCLabelBMFont::create("Stroke Width:", "goldFont.fnt");
    strokeWidthLabel->setPosition({ 280.0f, 110.0f });
    strokeWidthLabel->setScale(0.7f);
    strokeWidthLabel->setID("stroke-width-label");
    m_mainLayer->addChild(strokeWidthLabel);

    auto strokeWidthInput = TextInput::create(60.0f, "Num");
    strokeWidthInput->setPosition({ 365.0f, 110.0f });
    strokeWidthInput->setScale(0.6f);
    strokeWidthInput->setString(fmt::format("{:.1f}", m_trailInfo.stroke));
    strokeWidthInput->setFilter(".0123456789");
    strokeWidthInput->setMaxCharCount(4);
    strokeWidthInput->setCallback([this, strokeWidthSlider](const std::string& str) {
        #ifdef __cpp_lib_to_chars
        std::from_chars(str.data(), str.data() + str.size(), m_trailInfo.stroke);
        #else
        GEODE_UNWRAP_INTO_IF_OK(m_trailInfo.stroke, numFromString<float>(str));
        #endif
        m_trailInfo.stroke = roundf(std::clamp(m_trailInfo.stroke * 10.0f, 0.0f, 200.0f)) / 10.0f;
        strokeWidthSlider->setValue(m_trailInfo.stroke / 20.0f);
    });
    strokeWidthInput->setID("stroke-width-input");
    m_mainLayer->addChild(strokeWidthInput);

    CCMenuItemExt::assignCallback<SliderThumb>(strokeWidthSlider->m_touchLogic->m_thumb, [this, strokeWidthInput](SliderThumb* sender) {
        m_trailInfo.stroke = roundf(sender->getValue() * 200.0f) / 10.0f;
        strokeWidthInput->setString(fmt::format("{:.1f}", m_trailInfo.stroke));
    });

    //auto blendToggle = CCMenuItemToggler::createWithStandardSprites(nullptr, nullptr, 0.8f);
    auto blendToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.8f, [this](auto) {
        m_trailInfo.blend = !m_trailInfo.blend;
    });
    blendToggle->setPosition({ 60.0f, 60.0f });
    blendToggle->toggle(m_trailInfo.blend);
    blendToggle->setID("blend-toggle");
    m_buttonMenu->addChild(blendToggle);

    auto blendLabel = CCLabelBMFont::create("Additive\nBlending", "bigFont.fnt");
    blendLabel->setPosition({ 80.0f, 60.0f });
    blendLabel->setScale(0.4f);
    blendLabel->setAnchorPoint({ 0.0f, 0.5f });
    blendLabel->setID("blend-label");
    m_mainLayer->addChild(blendLabel);

    auto tintToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.8f, [this](auto) {
        m_trailInfo.tint = !m_trailInfo.tint;
    });
    tintToggle->setPosition({ 170.0f, 60.0f });
    tintToggle->toggle(m_trailInfo.tint);
    tintToggle->setID("tint-toggle");
    m_buttonMenu->addChild(tintToggle);

    auto tintLabel = CCLabelBMFont::create("Tint\nColor", "bigFont.fnt");
    tintLabel->setPosition({ 190.0f, 60.0f });
    tintLabel->setScale(0.4f);
    tintLabel->setAnchorPoint({ 0.0f, 0.5f });
    tintLabel->setID("tint-label");
    m_mainLayer->addChild(tintLabel);

    auto showToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.8f, [this](auto) {
        m_trailInfo.show = !m_trailInfo.show;
    });
    showToggle->setPosition({ 280.0f, 60.0f });
    showToggle->toggle(m_trailInfo.show);
    showToggle->setID("show-toggle");
    m_buttonMenu->addChild(showToggle);

    auto showLabel = CCLabelBMFont::create("Always\nShow", "bigFont.fnt");
    showLabel->setPosition({ 300.0f, 60.0f });
    showLabel->setScale(0.4f);
    showLabel->setAnchorPoint({ 0.0f, 0.5f });
    showLabel->setID("show-label");
    m_mainLayer->addChild(showLabel);

    auto saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", 0.8f), [this, info](auto) {
        info->trailInfo = m_trailInfo;
        onClose(nullptr);
    });
    saveButton->setPosition({ 200.0f, 25.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    return true;
}
