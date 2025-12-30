#include "SpecialSettingsPopup.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/file.hpp>
#include <IconInfo.hpp>
#include <jasmine/convert.hpp>

using namespace geode::prelude;

SpecialSettingsPopup* SpecialSettingsPopup::create(IconInfo* info) {
    auto ret = new SpecialSettingsPopup();
    auto type = info->getType();
    if (ret->init(info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SpecialSettingsPopup::init(IconInfo* info) {
    auto type = info->getType();
    if (!BasePopup::init(
        type == IconType::DeathEffect ? 500.0f : 400.0f, type == IconType::DeathEffect ? 280.0f : type == IconType::ShipFire ? 200.0f : 150.0f
    )) return false;

    setID("SpecialSettingsPopup");
    setTitle(fmt::format("{} Settings", Constants::getSingularUppercase(type)), "goldFont.fnt", 0.7f, 15.0f);
    m_title->setID("special-settings-title");

    m_settings = info->getSpecialInfo();

    if (type == IconType::DeathEffect) {
        addControl("scale", "Scale:", { 65.0f, 225.0f }, 0.7f, 0.0f, 2.0f, 1.0f, 2);
        addControl("scale-variance", "+-:", { 565.0f / 3.0f, 225.0f }, 0.7f, 0.0f, 1.0f, 0.0f, 2);
        addControl("rotation", "Rotation:", { 935.0f / 3.0f, 225.0f }, 0.7f, 0.0f, 360.0f, 0.0f, 0);
        addControl("rotation-variance", "+-:", { 435.0f, 225.0f }, 0.7f, 0.0f, 360.0f, 0.0f, 0);
        addToggle("blend", "Additive\nBlending", { 70.0f, 200.0f }, 1.0f, false);
        addControl("frame-delay", "Frame Delay:", { 250.0f, 192.0f }, 0.7f, 0.0f, 0.1f, 0.0f, 3);
        addControl("frame-delay-variance", "+-:", { 400.0f, 192.0f }, 0.7f, 0.0f, 0.1f, 0.0f, 3);
        addToggle("fade", "Fade\nOut", { 70.0f, 168.0f }, 1.0f, true);
        addControl("fade-delay-multiplier", "Fade Delay:", { 250.0f, 160.0f }, 0.7f, 0.0f, 20.0f, 6.0f, 1);
        addControl("fade-time-multiplier", "Fade Time:", { 400.0f, 160.0f }, 0.7f, 0.0f, 20.0f, 6.0f, 1);
        addLabel("circle-label", "Circle Effect", { 250.0f, 140.0f });
        addControl("circle-start-radius", "Start Radius:", { 65.0f, 105.0f }, 0.7f, 0.0f, 20.0f, 10.0f, 2);
        addControl("circle-end-radius", "End Radius:", { 565.0f / 3.0f, 105.0f }, 0.7f, 0.0f, 200.0f, 110.0f, 1);
        addToggle("circle-use-scale", "Use\nScale", { 262.5f, 113.0f }, 0.8f, false);
        addControl("circle-duration", "Duration:", { 373.5f, 105.0f }, 0.7f, 0.0f, 20.0f, 0.6f, 2);
        addToggle("circle-use-delay", "Use\nDelay", { 446.0f, 113.0f }, 0.8f, false);
        addLabel("outline-label", "Outline Effect", { 250.0f, 85.0f });
        addControl("outline-start-radius", "Start Radius:", { 65.0f, 50.0f }, 0.7f, 0.0f, 20.0f, 10.0f, 2);
        addControl("outline-end-radius", "End Radius:", { 565.0f / 3.0f, 50.0f }, 0.7f, 0.0f, 200.0f, 115.0f, 1);
        addToggle("outline-use-scale", "Use\nScale", { 262.5f, 58.0f }, 0.8f, false);
        addControl("outline-duration", "Duration:", { 373.5f, 50.0f }, 0.7f, 0.0f, 20.0f, 0.4f, 2);
        addToggle("outline-use-delay", "Use\nDelay", { 446.0f, 58.0f }, 0.8f, false);
    }
    else if (type == IconType::Special) {
        addControl("fade", "Fade Time:", { 100.0f, 87.5f }, 1.0f, 0.0f, 2.0f, 0.3f, 2);
        addControl("stroke", "Stroke Width:", { 300.0f, 87.5f }, 1.0f, 0.0f, 20.0f, 14.0f, 1);
        addToggle("blend", "Additive\nBlending", { 60.0f, 60.0f }, 1.0f, true);
        addToggle("tint", "Tint\nColor", { 170.0f, 60.0f }, 1.0f, false);
        addToggle("show", "Always\nShow", { 280.0f, 60.0f }, 1.0f, true);
    }
    else if (type == IconType::ShipFire) {
        addControl("fade", "Fade Time:", { 100.0f, 135.0f }, 1.0f, 0.0f, 2.0f, 0.1f, 2);
        addControl("stroke", "Stroke Width:", { 300.0f, 135.0f }, 1.0f, 0.0f, 40.0f, 20.0f, 1);
        addControl("x", "Offset X:", { 100.0f, 92.5f }, 1.0f, -20.0f, 20.0f, 0.0f, 1);
        addControl("y", "Offset Y:", { 300.0f, 92.5f }, 1.0f, -20.0f, 20.0f, 0.0f, 1);
        addControl("interval", "Fire Interval:", { 100.0f, 50.0f }, 1.0f, 0.0f, 1.0f, 0.5f, 2);
        addToggle("blend", "Additive\nBlending", { 280.0f, 60.0f }, 1.0f, true);
    }

    auto saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", 0.8f), [this, info](auto) {
        if (auto res = file::writeToJson(info->getJSON(), m_settings); res.isErr()) {
            Notify::error("Failed to save info: {}", res.unwrapErr());
        }
        else {
            info->setSpecialInfo(std::move(m_settings));
            onClose(nullptr);
        }
    });
    saveButton->setPosition({ m_size.width / 2.0f, 25.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}

void SpecialSettingsPopup::addControl(
    std::string_view id, const char* text, const CCPoint& position, float scale, float min, float max, float def, int decimals
) {
    auto& value = m_settings[id];
    if (!value.isNumber()) value = def;
    auto initial = value.as<float>().unwrapOr(def);
    auto factor = max - min;

    auto positioner = CCNode::create();
    positioner->setPosition(position);
    positioner->setScale(scale);
    positioner->setAnchorPoint({ 0.5f, 0.5f });
    positioner->setID(fmt::format("{}-positioner", id));
    m_mainLayer->addChild(positioner);

    auto slider = Slider::create(nullptr, nullptr, 0.8f);
    slider->setPosition({ 0.0f, 0.0f });
    slider->setValue((initial - min) / factor);
    slider->setID(fmt::format("{}-slider", id));
    positioner->addChild(slider);

    auto container = CCNode::create();
    container->setPosition({ 0.0f, 22.5f });
    container->setContentSize({ 170.0f, 30.0f });
    container->setAnchorPoint({ 0.5f, 0.5f });
    container->setLayout(RowLayout::create()->setAutoScale(false));
    container->setID(fmt::format("{}-container", id));
    positioner->addChild(container);

    auto label = CCLabelBMFont::create(text, "goldFont.fnt");
    label->setScale(0.7f);
    label->setID(fmt::format("{}-label", id));
    container->addChild(label);

    auto exponent = 1;
    for (int i = 0; i < decimals; i++) exponent *= 10;

    auto input = TextInput::create(60.0f, "Num");
    input->setScale(0.6f);
    input->setString(fmt::format("{:.{}f}", initial, decimals));
    input->setFilter(std::string(min < 0.0f ? "-.0123456789" : ".0123456789", min < 0.0f ? 12 : 11));
    input->setMaxCharCount(fmt::to_string(max).size() + decimals + (min < 0.0f ? 2 : 1));
    input->setCallback([this, def, exponent, min, max, slider, &value](const std::string& str) {
        auto floatValue = value.as<float>().unwrapOr(def);
        jasmine::convert::to(str, floatValue);
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

void SpecialSettingsPopup::addToggle(std::string_view id, const char* label, const CCPoint& position, float scale, bool def) {
    auto& value = m_settings[id];
    if (!value.isBool()) value = def;

    auto positioner = CCMenu::create();
    positioner->setPosition(position);
    positioner->setScale(scale);
    positioner->setContentSize({ 0.0f, 0.0f });
    positioner->setAnchorPoint({ 0.5f, 0.5f });
    positioner->ignoreAnchorPointForPosition(false);
    positioner->setID(fmt::format("{}-positioner", id));
    m_mainLayer->addChild(positioner);

    auto toggle = CCMenuItemExt::createTogglerWithStandardSprites(0.8f, [this, def, &value](auto) {
        value = !value.asBool().unwrapOr(def);
    });
    toggle->setPosition({ 0.0f, 0.0f });
    toggle->toggle(value.asBool().unwrapOr(def));
    toggle->setID(fmt::format("{}-toggle", id));
    positioner->addChild(toggle);

    auto toggleLabel = CCLabelBMFont::create(label, "bigFont.fnt");
    toggleLabel->setPosition({ 20.0f, 0.0f });
    toggleLabel->setScale(0.4f);
    toggleLabel->setAnchorPoint({ 0.0f, 0.5f });
    toggleLabel->setID(fmt::format("{}-label", id));
    positioner->addChild(toggleLabel);
}

void SpecialSettingsPopup::addLabel(std::string&& id, const char* text, const CCPoint& position) {
    auto label = CCLabelBMFont::create(text, "goldFont.fnt");
    label->setPosition(position);
    label->setScale(0.7f);
    label->setID(std::move(id));
    m_mainLayer->addChild(label);
}
