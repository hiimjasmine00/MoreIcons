#include "ButtonHooker.hpp"

using namespace geode::prelude;

ButtonHooker* ButtonHooker::create(CCMenuItem* button, CCObject* listener, SEL_MenuHandler selector) {
    auto hooker = new ButtonHooker();
    hooker->m_listener = button->m_pListener;
    hooker->m_selector = button->m_pfnSelector;
    button->setTarget(listener, selector);
    button->setUserObject(GEODE_MOD_ID "/hooker", hooker);
    hooker->autorelease();
    return hooker;
}
