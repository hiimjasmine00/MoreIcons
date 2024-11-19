#define CALL_BUTTON_ORIGINAL(button) \
    if (auto hooker = static_cast<ButtonHooker*>(static_cast<CCNode*>(button)->getUserObject("hooker"_spr))) \
        (hooker->m_listener->*hooker->m_selector)(button)

class ButtonHooker : public cocos2d::CCObject {
public:
    cocos2d::CCObject* m_listener;
    cocos2d::SEL_MenuHandler m_selector;

    static ButtonHooker* create(cocos2d::CCMenuItem* button, cocos2d::CCObject* listener, cocos2d::SEL_MenuHandler selector);
};
