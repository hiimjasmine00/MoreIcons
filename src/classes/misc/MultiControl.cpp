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
    m_minimum = min;
    m_maximum = max;
    m_default = def;
    m_decimals = decimals;
    m_exponent = 1;
    for (int i = 0; i < decimals; i++) m_exponent *= 10;

    m_slider = Slider::create(this, menu_selector(MultiControl::sliderChanged), sliderScale);
    m_slider->setValue((value - min) / (max - min));
    m_slider->setID("control-slider");
    addChild(m_slider);

    m_menu = CCMenu::create();
    m_menu->setPosition({ 0.0f, 0.0f });
    m_menu->setContentSize({ m_slider->m_groove->getTextureRect().size.width * sliderScale, 30.0f });
    m_menu->ignoreAnchorPointForPosition(false);
    m_menu->setLayout(RowLayout::create()->setAutoScale(false), false);
    m_menu->setID("control-menu");
    addChild(m_menu);

    m_label = CCLabelBMFont::create(text, "goldFont.fnt");
    m_label->setID("title-label");
    m_menu->addChild(m_label);

    m_input = TextInput::create(inputWidth, "Num");
    m_input->setFilter(min < 0.0f ?
        (decimals > 0 ? GDSTR("-.0123456789") : GDSTR("-0123456789")) : (decimals > 0 ? GDSTR(".0123456789") : GDSTR("0123456789")));
    m_input->setMaxCharCount((int)log10f(std::max(floorf(max), 1.0f)) + 1 + decimals + (int)(decimals > 0) + (int)(min < 0.0f));
    m_input->setString(fmt::format("{:.{}f}", value, decimals));
    m_input->setDelegate(this);
    m_input->setID("value-input");
    m_menu->addChild(m_input);

    auto resetSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    resetSprite->setScale(0.4f);
    m_resetButton = CCMenuItemSpriteExtra::create(resetSprite, this, menu_selector(MultiControl::onReset));
    m_resetButton->setID("reset-button");
    m_menu->addChild(m_resetButton);

    return true;
}

void MultiControl::sliderChanged(CCObject* sender) {
    auto value = static_cast<SliderThumb*>(sender)->getValue() * (m_maximum - m_minimum) + m_minimum;
    value = std::clamp(roundf(value * m_exponent) / m_exponent, m_minimum, m_maximum);
    m_input->setString(fmt::format("{:.{}f}", value, m_decimals));
    if (m_onChange) m_onChange(value);
}

void MultiControl::textChanged(CCTextInputNode* input) {
    if (auto valueRes = numFromString<float>(*input->m_textField->m_pInputText)) {
        auto value = valueRes.unwrap();
        value = std::clamp(roundf(value * m_exponent) / m_exponent, m_minimum, m_maximum);
        m_slider->setValue((value - m_minimum) / (m_maximum - m_minimum));
        if (m_onChange) m_onChange(value);
    }
}

void MultiControl::onReset(CCObject* sender) {
    m_slider->setValue((m_default - m_minimum) / (m_maximum - m_minimum));
    m_input->setString(fmt::format("{:.{}f}", m_default, m_decimals));
    if (m_onChange) m_onChange(m_default);
}

void MultiControl::setValue(float value) {
    value = std::clamp(roundf(value * m_exponent) / m_exponent, m_minimum, m_maximum);
    m_slider->setValue((value - m_minimum) / (m_maximum - m_minimum));
    m_input->setString(fmt::format("{:.{}f}", value, m_decimals));
}
