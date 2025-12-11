#include "SpecialSettingsPopup.hpp"
#include "../../../MoreIcons.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/file.hpp>
#include <jasmine/convert.hpp>

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

    auto trailInfo = info->getSpecialInfo();
    m_blend = trailInfo.get<bool>("blend").unwrapOr(false);
    m_tint = trailInfo.get<bool>("tint").unwrapOr(false);
    m_show = trailInfo.get<bool>("show").unwrapOr(false);
    m_fade = trailInfo.get<float>("fade").unwrapOr(0.3f);
    m_stroke = trailInfo.get<float>("stroke").unwrapOr(14.0f);

    auto fadeTimeSlider = Slider::create(nullptr, nullptr, 0.8f);
    fadeTimeSlider->setPosition({ 100.0f, 87.5f });
    fadeTimeSlider->setValue(roundf(m_fade * 50.0f) / 100.0f);
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
    fadeTimeInput->setString(fmt::format("{:.2f}", m_fade));
    fadeTimeInput->setFilter(".0123456789");
    fadeTimeInput->setMaxCharCount(4);
    fadeTimeInput->setCallback([this, fadeTimeSlider](const std::string& str) {
        jasmine::convert::toFloat(str, m_fade);
        m_fade = std::clamp(m_fade * 100.0f, 0.0f, 200.0f) / 100.0f;
        fadeTimeSlider->setValue(m_fade / 2.0f);
    });
    fadeTimeInput->setID("fade-time-input");
    m_mainLayer->addChild(fadeTimeInput);

    CCMenuItemExt::assignCallback<SliderThumb>(fadeTimeSlider->m_touchLogic->m_thumb, [this, fadeTimeInput](SliderThumb* sender) {
        m_fade = roundf(sender->getValue() * 200.0f) / 100.0f;
        fadeTimeInput->setString(fmt::format("{:.2f}", m_fade));
    });

    auto strokeWidthSlider = Slider::create(nullptr, nullptr, 0.8f);
    strokeWidthSlider->setPosition({ 300.0f, 87.5f });
    strokeWidthSlider->setValue(roundf(m_stroke / 2.0f) / 10.0f);
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
    strokeWidthInput->setString(fmt::format("{:.1f}", m_stroke));
    strokeWidthInput->setFilter(".0123456789");
    strokeWidthInput->setMaxCharCount(4);
    strokeWidthInput->setCallback([this, strokeWidthSlider](const std::string& str) {
        jasmine::convert::toFloat(str, m_stroke);
        m_stroke = roundf(std::clamp(m_stroke * 10.0f, 0.0f, 200.0f)) / 10.0f;
        strokeWidthSlider->setValue(m_stroke / 20.0f);
    });
    strokeWidthInput->setID("stroke-width-input");
    m_mainLayer->addChild(strokeWidthInput);

    CCMenuItemExt::assignCallback<SliderThumb>(strokeWidthSlider->m_touchLogic->m_thumb, [this, strokeWidthInput](SliderThumb* sender) {
        m_stroke = roundf(sender->getValue() * 200.0f) / 10.0f;
        strokeWidthInput->setString(fmt::format("{:.1f}", m_stroke));
    });

    auto blendToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.8f, [this](auto) {
        m_blend = !m_blend;
    });
    blendToggle->setPosition({ 60.0f, 60.0f });
    blendToggle->toggle(m_blend);
    blendToggle->setID("blend-toggle");
    m_buttonMenu->addChild(blendToggle);

    auto blendLabel = CCLabelBMFont::create("Additive\nBlending", "bigFont.fnt");
    blendLabel->setPosition({ 80.0f, 60.0f });
    blendLabel->setScale(0.4f);
    blendLabel->setAnchorPoint({ 0.0f, 0.5f });
    blendLabel->setID("blend-label");
    m_mainLayer->addChild(blendLabel);

    auto tintToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.8f, [this](auto) {
        m_tint = !m_tint;
    });
    tintToggle->setPosition({ 170.0f, 60.0f });
    tintToggle->toggle(m_tint);
    tintToggle->setID("tint-toggle");
    m_buttonMenu->addChild(tintToggle);

    auto tintLabel = CCLabelBMFont::create("Tint\nColor", "bigFont.fnt");
    tintLabel->setPosition({ 190.0f, 60.0f });
    tintLabel->setScale(0.4f);
    tintLabel->setAnchorPoint({ 0.0f, 0.5f });
    tintLabel->setID("tint-label");
    m_mainLayer->addChild(tintLabel);

    auto showToggle = CCMenuItemExt::createTogglerWithStandardSprites(0.8f, [this](auto) {
        m_show = !m_show;
    });
    showToggle->setPosition({ 280.0f, 60.0f });
    showToggle->toggle(m_show);
    showToggle->setID("show-toggle");
    m_buttonMenu->addChild(showToggle);

    auto showLabel = CCLabelBMFont::create("Always\nShow", "bigFont.fnt");
    showLabel->setPosition({ 300.0f, 60.0f });
    showLabel->setScale(0.4f);
    showLabel->setAnchorPoint({ 0.0f, 0.5f });
    showLabel->setID("show-label");
    m_mainLayer->addChild(showLabel);

    auto saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", 0.8f), [this, info](auto) {
        auto trailInfo = matjson::makeObject({
            { "blend", m_blend },
            { "tint", m_tint },
            { "show", m_show },
            { "fade", m_fade },
            { "stroke", m_stroke }
        });
        if (auto res = file::writeToJson(info->getJSON(), trailInfo); res.isErr()) {
            MoreIcons::notifyFailure("Failed to save trail info: {}", res.unwrapErr());
        }
        else {
            info->moveSpecialInfo(std::move(trailInfo));
            onClose(nullptr);
        }
    });
    saveButton->setPosition({ 200.0f, 25.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}
