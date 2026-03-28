#include "SpecialSettingsPopup.hpp"
#include "../../misc/MultiControl.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Defaults.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/utils/file.hpp>
#include <IconInfo.hpp>

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
    m_info = info;
    m_iconType = type;

    if (type == IconType::DeathEffect) {
        checkDefaults(Defaults::getDeathEffectInfo(0));

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
        checkDefaults(Defaults::getTrailInfo(0));

        addControl("fade", "Fade Time:", { 100.0f, 87.5f }, 1.0f, 0.0f, 2.0f, 0.3f, 2);
        addControl("stroke", "Stroke Width:", { 300.0f, 87.5f }, 1.0f, 0.0f, 20.0f, 14.0f, 1);
        addToggle("blend", "Additive\nBlending", { 60.0f, 60.0f }, 1.0f, true);
        addToggle("tint", "Tint\nColor", { 170.0f, 60.0f }, 1.0f, false);
        addToggle("show", "Always\nShow", { 280.0f, 60.0f }, 1.0f, true);
    }
    else if (type == IconType::ShipFire) {
        checkDefaults(Defaults::getShipFireInfo(0));

        addControl("fade", "Fade Time:", { 100.0f, 135.0f }, 1.0f, 0.0f, 2.0f, 0.1f, 2);
        addControl("stroke", "Stroke Width:", { 300.0f, 135.0f }, 1.0f, 0.0f, 40.0f, 20.0f, 1);
        addControl("x", "Offset X:", { 100.0f, 92.5f }, 1.0f, -20.0f, 20.0f, 0.0f, 1);
        addControl("y", "Offset Y:", { 300.0f, 92.5f }, 1.0f, -20.0f, 20.0f, 0.0f, 1);
        addControl("interval", "Fire Interval:", { 100.0f, 50.0f }, 1.0f, 0.0f, 1.0f, 0.5f, 2);
        addToggle("blend", "Additive\nBlending", { 280.0f, 60.0f }, 1.0f, true);
    }

    auto saveButton = CCMenuItemSpriteExtra::create(ButtonSprite::create("Save", 0.8f), this, menu_selector(SpecialSettingsPopup::onSave));
    saveButton->setPosition({ m_size.width / 2.0f, 25.0f });
    saveButton->setID("save-button");
    m_buttonMenu->addChild(saveButton);

    handleTouchPriority(this);

    return true;
}

void SpecialSettingsPopup::checkDefaults(const matjson::Value& defaultInfo) {
    for (auto& value : defaultInfo) {
        auto key = value.getKey().value_or(std::string());
        if (auto valueRes = m_settings.get(key)) {
            if (valueRes.unwrap().type() != value.type()) m_settings.set(key, value);
        }
        else {
            m_settings.set(key, value);
        }
    }
}

void SpecialSettingsPopup::onSave(CCObject* sender) {
    if (auto res = file::writeString(m_info->getJSON(), m_settings.dump()); res.isErr()) {
        Notify::error("Failed to save info: {}", res.unwrapErr());
    }
    else {
        m_info->setSpecialInfo(std::move(m_settings));
        close();
    }
}

void SpecialSettingsPopup::addControl(
    std::string_view id, const char* text, const CCPoint& position, float scale, float min, float max, float def, int decimals
) {
    auto& val = m_settings[id];

    auto multiControl = MultiControl::create([this, &val](float value) {
        val = value;
        m_hasChanged = true;
    }, text, val.as<float>().unwrapOr(def), min, max, def, decimals, 0.8f, 60.0f);
    multiControl->setPosition(position);
    multiControl->setScale(scale);
    multiControl->setID(fmt::format("{}-control", id));
    m_mainLayer->addChild(multiControl);

    multiControl->getLabel()->setScale(0.7f);
    multiControl->getInput()->setScale(0.6f);
    multiControl->getResetButton()->removeFromParent();

    auto menu = multiControl->getMenu();
    menu->setPosition({ 0.0f, 22.5f });
    menu->updateLayout();
}

void SpecialSettingsPopup::addToggle(std::string_view id, const char* label, const CCPoint& position, float scale, bool def) {
    auto positioner = CCMenu::create();
    positioner->setPosition(position);
    positioner->setScale(scale);
    positioner->setContentSize({ 0.0f, 0.0f });
    positioner->setAnchorPoint({ 0.5f, 0.5f });
    positioner->ignoreAnchorPointForPosition(false);
    positioner->setID(fmt::format("{}-positioner", id));
    m_mainLayer->addChild(positioner);

    auto offSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    offSprite->setScale(0.8f);
    auto onSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    onSprite->setScale(0.8f);
    auto toggle = CCMenuItemToggler::create(offSprite, onSprite, this, menu_selector(SpecialSettingsPopup::onToggle));
    toggle->toggle(m_settings[id].asBool().unwrapOr(def));
    toggle->setID(fmt::format("{}-toggle", id));
    positioner->addChild(toggle);

    auto toggleLabel = CCLabelBMFont::create(label, "bigFont.fnt");
    toggleLabel->setPosition({ 20.0f, 0.0f });
    toggleLabel->setScale(0.4f);
    toggleLabel->setAnchorPoint({ 0.0f, 0.5f });
    toggleLabel->setID(fmt::format("{}-label", id));
    positioner->addChild(toggleLabel);
}

void SpecialSettingsPopup::onToggle(CCObject* sender) {
    auto toggle = static_cast<CCMenuItemToggler*>(sender);
    auto id = toggle->getID().view();
    id.remove_suffix(7);
    m_settings[id] = !toggle->m_toggled;
    m_hasChanged = true;
}

void SpecialSettingsPopup::addLabel(std::string&& id, const char* text, const CCPoint& position) {
    auto label = CCLabelBMFont::create(text, "goldFont.fnt");
    label->setPosition(position);
    label->setScale(0.7f);
    label->setID(std::move(id));
    m_mainLayer->addChild(label);
}

void SpecialSettingsPopup::onClose(CCObject* sender) {
    if (!m_hasChanged) return close();

    auto type = m_iconType;
    FLAlertLayer::create(
        this,
        fmt::format("Exit {} Settings Editor", Constants::getSingularUppercase(type)).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} settings editor</c>?\n<cr>All unsaved changes will be lost!</c>",
            Constants::getSingularLowercase(type)),
        "No",
        "Yes",
        430.0f
    )->show();
}

void SpecialSettingsPopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (btn2) close();
}
