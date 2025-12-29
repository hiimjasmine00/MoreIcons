#include <Geode/modify/GJRobotSprite.hpp>

using namespace geode::prelude;

class $modify(MIRobotSprite, GJRobotSprite) {
    void updateFrame(int frame) {
        GJRobotSprite::updateFrame(frame);

        if (auto str = static_cast<CCString*>(getUserObject("name"_spr))) {
            str->m_sString.clear();
        }
    }
};
