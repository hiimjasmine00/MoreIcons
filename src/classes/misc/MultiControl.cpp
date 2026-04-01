#include "MultiControl.hpp"
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/ui/Layout.hpp>
#include <Geode/utils/general.hpp>

using namespace geode::prelude;

#ifdef GEODE_IS_ANDROID
#define GDSTR(x) gd::string(x, sizeof(x) - 1)
#else
#define GDSTR(x) std::string(x, sizeof(x) - 1)
#endif

MultiControl* MultiControl::create(
    Function<void(float)> onChange, const char* text, float value, float min, float max, float def, int decimals,
    float sliderScale, float inputWidth
) {
    auto ret = new MultiControl();
    if (ret->init(std::move(onChange), text, value, min, max, def, decimals, sliderScale, inputWidth)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MultiControl::init(
    Function<void(float)> onChange, const char* text, float value, float min, float max, float def, int decimals,
    float sliderScale, float inputWidth
) {
    if (!CCNode::init()) return false;

    m_onChange = std::move(onChange);
    auto exponent = 1;
    for (int i = 0; i < decimals; i++) exponent *= 10;

    m_slider = SliderNode::create(nullptr);
    m_slider->setScale(sliderScale);
    m_slider->setMin(min);
    m_slider->setMax(max);
    m_slider->setLabelPrecision(decimals);
    m_slider->setTextInputPrecision(decimals);
    m_slider->setValue(value);
    m_slider->setID("control-slider");
    addChild(m_slider);

    m_menu = CCMenu::create();
    m_menu->setPosition({ 0.0f, 0.0f });
    m_menu->setContentSize({ m_slider->getGroove()->getContentWidth() * sliderScale, 30.0f });
    m_menu->ignoreAnchorPointForPosition(false);
    m_menu->setLayout(RowLayout::create()->setAutoScale(false), false);
    m_menu->setID("control-menu");
    addChild(m_menu);

    m_label = CCLabelBMFont::create(text, "goldFont.fnt");
    m_label->setID("title-label");
    m_menu->addChild(m_label);

    m_input = TextInput::create(inputWidth, "Num");
    m_slider->linkTextInput(m_input, decimals);
    m_input->setFilter(min < 0.0f ?
        (decimals > 0 ? GDSTR("-.0123456789") : GDSTR("-0123456789")) : (decimals > 0 ? GDSTR(".0123456789") : GDSTR("0123456789")));
    m_input->setMaxCharCount((int)log10f(std::max(floorf(max), 1.0f)) + 1 + decimals + (int)(decimals > 0) + (int)(min < 0.0f));
    m_input->setString(fmt::format("{:.{}f}", value, decimals));
    m_input->setID("value-input");
    m_menu->addChild(m_input);

    auto resetSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    resetSprite->setScale(0.4f);
    m_resetButton = CCMenuItemSpriteExtra::create(resetSprite, this, menu_selector(MultiControl::onReset));
    m_resetButton->setID("reset-button");
    m_menu->addChild(m_resetButton);

    m_slider->setSlideCallback([this, exponent](SliderNode* slider, float value) {
        if (m_onChange) m_onChange(roundf(value * exponent) / exponent);
    });

    return true;
}

void MultiControl::onReset(CCObject* sender) {
    setValue(m_default);
}

void MultiControl::setValue(float value) {
    m_slider->setValue(value);
}
