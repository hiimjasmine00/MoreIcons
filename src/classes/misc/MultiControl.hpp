#include <Geode/ui/SliderNode.hpp>
#include <Geode/ui/TextInput.hpp>

class MultiControl : public cocos2d::CCNode {
protected:
    geode::Function<void(float)> m_onChange;
    geode::SliderNode* m_slider;
    cocos2d::CCMenu* m_menu;
    geode::TextInput* m_input;
    cocos2d::CCLabelBMFont* m_label;
    CCMenuItemSpriteExtra* m_resetButton;
    float m_minimum;
    float m_maximum;
    float m_default;
    int m_decimals;
    int m_exponent;

    bool init(
        geode::Function<void(float)> onChange, const char* text, float value, float min, float max, float def, int decimals,
        float sliderScale, float inputWidth
    );
public:
    static MultiControl* create(
        geode::Function<void(float)> onChange, const char* text, float value, float min, float max, float def, int decimals,
        float sliderScale, float inputWidth
    );

    void onReset(CCObject* sender);

    geode::SliderNode* getSlider() const { return m_slider; }
    cocos2d::CCMenu* getMenu() const { return m_menu; }
    geode::TextInput* getInput() const { return m_input; }
    cocos2d::CCLabelBMFont* getLabel() const { return m_label; }
    CCMenuItemSpriteExtra* getResetButton() const { return m_resetButton; }

    void setValue(float value);
};
