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
    auto type = info->getType();
    if (ret->initAnchored(400.0f, type == IconType::ShipFire ? 200.0f : 150.0f, info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SpecialSettingsPopup::setup(IconInfo* info) {
    auto type = info->getType();

    setID("SpecialSettingsPopup");
    setTitle(fmt::format("{} Settings", MoreIcons::uppercase[MoreIcons::convertType(type)]), "goldFont.fnt", 0.7f, 15.0f);
    m_title->setID("special-settings-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_info = info->getSpecialInfo();

    if (type == IconType::Special) {
        addControl("fade", "Fade Time:", { 100.0f, 87.5f }, 0.0f, 2.0f, 0.3f, 2);
        addControl("stroke", "Stroke Width:", { 300.0f, 87.5f }, 0.0f, 20.0f, 14.0f, 1);
        addToggle("blend", "Additive\nBlending", { 60.0f, 60.0f }, false);
        addToggle("tint", "Tint\nColor", { 170.0f, 60.0f }, false);
        addToggle("show", "Always\nShow", { 280.0f, 60.0f }, true);
    }
    else if (type == IconType::ShipFire) {
        addControl("fade", "Fade Time:", { 100.0f, 135.0f }, 0.0f, 2.0f, 0.1f, 2);
        addControl("stroke", "Stroke Width:", { 300.0f, 135.0f }, 0.0f, 40.0f, 20.0f, 1);
        addControl("x", "Offset X:", { 100.0f, 92.5f }, -20.0f, 20.0f, 0.0f, 1);
        addControl("y", "Offset Y:", { 300.0f, 92.5f }, -20.0f, 20.0f, 0.0f, 1);
        addControl("interval", "Fire Interval:", { 100.0f, 50.0f }, 0.0f, 1.0f, 0.5f, 2);
        addToggle("blend", "Additive\nBlending", { 280.0f, 60.0f }, true);
    }

    auto saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", 0.8f), [this, info](auto) {
        if (auto res = file::writeToJson(info->getJSON(), m_info); res.isErr()) {
            MoreIcons::notifyFailure("Failed to save info: {}", res.unwrapErr());
        }
        else {
            info->setSpecialInfo(std::move(m_info));
            onClose(nullptr);
        }
    });
    saveButton->setPosition({ 200.0f, 25.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}

void SpecialSettingsPopup::addControl(
    std::string_view id, const char* text, const CCPoint& position, float min, float max, float def, int decimals
) {
    auto& value = m_info[id];
    if (!value.isNumber()) value = def;
    auto initial = value.as<float>().unwrapOr(def);
    auto factor = max - min;

    auto slider = Slider::create(nullptr, nullptr, 0.8f);
    slider->setPosition(position);
    slider->setValue((initial - min) / factor);
    slider->setID(fmt::format("{}-slider", id));
    m_mainLayer->addChild(slider);

    auto container = CCNode::create();
    container->setPosition(position + CCPoint { 0.0f, 22.5f });
    container->setContentSize({ 170.0f, 30.0f });
    container->setAnchorPoint({ 0.5f, 0.5f });
    container->setLayout(RowLayout::create()->setAutoScale(false));
    container->setID(fmt::format("{}-container", id));
    m_mainLayer->addChild(container);

    auto label = CCLabelBMFont::create(text, "goldFont.fnt");
    label->setScale(0.7f);
    label->setID(fmt::format("{}-label", id));
    container->addChild(label);

    auto exponent = 1;
    for (int i = 0; i < decimals; i++) exponent *= 10;

    auto input = TextInput::create(60.0f, "Num");
    input->setScale(0.6f);
    input->setString(fmt::format("{:.{}f}", initial, decimals));
    input->setFilter(min < 0.0f ? "-.0123456789" : ".0123456789");
    input->setMaxCharCount(fmt::to_string(max).size() + decimals + (min < 0.0f ? 2 : 1));
    input->setCallback([this, def, exponent, min, max, slider, &value](const std::string& str) {
        auto floatValue = value.as<float>().unwrapOr(def);
        jasmine::convert::toFloat(str, floatValue);
        floatValue = std::clamp(roundf(floatValue * exponent) / exponent, min, max);
        slider->setValue((floatValue - min) / (max - min));
        value = floatValue;
    });
    input->setID(fmt::format("{}-input", id));
    container->addChild(input);

    container->updateLayout();

    CCMenuItemExt::assignCallback<SliderThumb>(slider->m_touchLogic->m_thumb, [
        this, def, decimals, exponent, input, min, max, &value
    ](SliderThumb* sender) {
        auto floatValue = (roundf(sender->getValue() * (max - min) + min) * exponent) / exponent;
        input->setString(fmt::format("{:.{}f}", floatValue, decimals));
        value = floatValue;
    });
}

void SpecialSettingsPopup::addToggle(std::string_view id, const char* label, const cocos2d::CCPoint& position, bool def) {
    auto& value = m_info[id];
    if (!value.isBool()) value = def;

    auto toggle = CCMenuItemExt::createTogglerWithStandardSprites(0.8f, [this, def, &value](auto) {
        value = !value.asBool().unwrapOr(def);
    });
    toggle->setPosition(position);
    toggle->toggle(value.asBool().unwrapOr(def));
    toggle->setID(fmt::format("{}-toggle", id));
    m_buttonMenu->addChild(toggle);

    auto toggleLabel = CCLabelBMFont::create(label, "bigFont.fnt");
    toggleLabel->setPosition(position + CCPoint { 20.0f, 0.0f });
    toggleLabel->setScale(0.4f);
    toggleLabel->setAnchorPoint({ 0.0f, 0.5f });
    toggleLabel->setID(fmt::format("{}-label", id));
    m_mainLayer->addChild(toggleLabel);
}
