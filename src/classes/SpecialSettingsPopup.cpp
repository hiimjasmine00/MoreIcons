#include "SpecialSettingsPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/ui/TextInput.hpp>

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

    m_fadeTime = info->fade;
    m_strokeWidth = info->stroke;

    auto fadeTimeSlider = Slider::create(nullptr, nullptr, 0.8f);
    fadeTimeSlider->setPosition({ 100.0f, 87.5f });
    fadeTimeSlider->setValue(roundf(m_fadeTime * 50.0f) / 100.0f);
    fadeTimeSlider->setID("fade-time-slider");
    m_mainLayer->addChild(fadeTimeSlider);

    auto fadeTimeLabel = CCLabelBMFont::create("Fade Time:", "goldFont.fnt");
    fadeTimeLabel->setPosition({ 80.0f, 110.0f });
    fadeTimeLabel->setScale(0.7f);
    fadeTimeLabel->setID("fade-time-label");
    m_mainLayer->addChild(fadeTimeLabel);

    auto fadeTimeInput = TextInput::create(60.0f, "Num", "bigFont.fnt");
    fadeTimeInput->setPosition({ 150.0f, 110.0f });
    fadeTimeInput->setScale(0.6f);
    fadeTimeInput->setString(fmt::format("{:.2f}", m_fadeTime));
    fadeTimeInput->setCommonFilter(CommonFilter::Float);
    fadeTimeInput->setMaxCharCount(4);
    fadeTimeInput->setCallback([this, fadeTimeSlider](const std::string& str) {
        m_fadeTime = roundf(std::clamp(numFromString<float>(str).unwrapOr(0.0f) * 100.0f, 0.0f, 200.0f)) / 100.0f;
        fadeTimeSlider->setValue(m_fadeTime / 2.0f);
    });
    fadeTimeInput->setID("fade-time-input");
    m_mainLayer->addChild(fadeTimeInput);

    CCMenuItemExt::assignCallback<SliderThumb>(fadeTimeSlider->m_touchLogic->m_thumb, [this, fadeTimeInput](SliderThumb* sender) {
        m_fadeTime = roundf(sender->getValue() * 200.0f) / 100.0f;
        fadeTimeInput->setString(fmt::format("{:.2f}", m_fadeTime));
    });

    auto strokeWidthSlider = Slider::create(nullptr, nullptr, 0.8f);
    strokeWidthSlider->setPosition({ 300.0f, 87.5f });
    strokeWidthSlider->setValue(roundf(m_strokeWidth / 2.0f) / 10.0f);
    strokeWidthSlider->m_touchLogic->m_thumb->setTag(2);
    strokeWidthSlider->setID("stroke-width-slider");
    m_mainLayer->addChild(strokeWidthSlider);

    auto strokeWidthLabel = CCLabelBMFont::create("Stroke Width:", "goldFont.fnt");
    strokeWidthLabel->setPosition({ 280.0f, 110.0f });
    strokeWidthLabel->setScale(0.7f);
    strokeWidthLabel->setID("stroke-width-label");
    m_mainLayer->addChild(strokeWidthLabel);

    auto strokeWidthInput = TextInput::create(60.0f, "Num", "bigFont.fnt");
    strokeWidthInput->setPosition({ 365.0f, 110.0f });
    strokeWidthInput->setScale(0.6f);
    strokeWidthInput->setString(fmt::format("{:.1f}", m_strokeWidth));
    strokeWidthInput->setCommonFilter(CommonFilter::Float);
    strokeWidthInput->setMaxCharCount(4);
    strokeWidthInput->setCallback([this, strokeWidthSlider](const std::string& str) {
        m_strokeWidth = roundf(std::clamp(numFromString<float>(str).unwrapOr(0.0f) * 10.0f, 0.0f, 200.0f)) / 10.0f;
        strokeWidthSlider->setValue(m_strokeWidth / 20.0f);
    });
    strokeWidthInput->setID("stroke-width-input");
    m_mainLayer->addChild(strokeWidthInput);

    CCMenuItemExt::assignCallback<SliderThumb>(strokeWidthSlider->m_touchLogic->m_thumb, [this, strokeWidthInput](SliderThumb* sender) {
        m_strokeWidth = roundf(sender->getValue() * 200.0f) / 10.0f;
        strokeWidthInput->setString(fmt::format("{:.1f}", m_strokeWidth));
    });

    auto blendToggle = CCMenuItemToggler::createWithStandardSprites(nullptr, nullptr, 0.8f);
    blendToggle->setPosition({ 60.0f, 60.0f });
    blendToggle->toggle(info->blend);
    blendToggle->setID("blend-toggle");
    m_buttonMenu->addChild(blendToggle);

    auto blendLabel = CCLabelBMFont::create("Additive\nBlending", "bigFont.fnt");
    blendLabel->setPosition({ 80.0f, 60.0f });
    blendLabel->setScale(0.4f);
    blendLabel->setAnchorPoint({ 0.0f, 0.5f });
    blendLabel->setID("blend-label");
    m_mainLayer->addChild(blendLabel);

    auto tintToggle = CCMenuItemToggler::createWithStandardSprites(nullptr, nullptr, 0.8f);
    tintToggle->setPosition({ 170.0f, 60.0f });
    tintToggle->toggle(info->tint);
    tintToggle->setID("tint-toggle");
    m_buttonMenu->addChild(tintToggle);

    auto tintLabel = CCLabelBMFont::create("Tint\nColor", "bigFont.fnt");
    tintLabel->setPosition({ 190.0f, 60.0f });
    tintLabel->setScale(0.4f);
    tintLabel->setAnchorPoint({ 0.0f, 0.5f });
    tintLabel->setID("tint-label");
    m_mainLayer->addChild(tintLabel);

    auto showToggle = CCMenuItemToggler::createWithStandardSprites(nullptr, nullptr, 0.8f);
    showToggle->setPosition({ 280.0f, 60.0f });
    showToggle->toggle(info->show);
    showToggle->setID("show-toggle");
    m_buttonMenu->addChild(showToggle);

    auto showLabel = CCLabelBMFont::create("Always\nShow", "bigFont.fnt");
    showLabel->setPosition({ 300.0f, 60.0f });
    showLabel->setScale(0.4f);
    showLabel->setAnchorPoint({ 0.0f, 0.5f });
    showLabel->setID("show-label");
    m_mainLayer->addChild(showLabel);

    auto saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", 0, 0, 0.8f, false, "goldFont.fnt", "GJ_button_01.png", 0.0f), [
        this, blendToggle, tintToggle, showToggle, info
    ](auto) {
        info->fade = m_fadeTime;
        info->stroke = m_strokeWidth;
        info->blend = blendToggle->m_toggled;
        info->tint = tintToggle->m_toggled;
        info->show = showToggle->m_toggled;
        onClose(nullptr);
    });
    saveButton->setPosition({ 200.0f, 25.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    return true;
}
