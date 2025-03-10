#include "../api/MoreIconsAPI.hpp"
#include <Geode/modify/GJRobotSprite.hpp>

using namespace geode::prelude;

class $modify(MIRobotSprite, GJRobotSprite) {
    struct Fields {
        GJRobotSprite* m_this;
        ~Fields() {
            if (!m_this) return;

            auto name = static_cast<CCString*>(m_this->getUserObject("name"_spr));
            if (!name) return;

            auto type = static_cast<CCInteger*>(m_this->getUserObject("type"_spr));
            if (!type) return;

            MoreIconsAPI::unloadIcon(name->m_sString, (IconType)type->m_nValue);

            m_this = nullptr;
        }
    };

    void updateFrame(int frame) {
        GJRobotSprite::updateFrame(frame);

        m_fields->m_this = this;

        setUserObject("name"_spr, nullptr);
        setUserObject("type"_spr, nullptr);
    }
};
