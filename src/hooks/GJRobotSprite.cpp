#include "../api/MoreIconsAPI.hpp"
#include <Geode/modify/GJRobotSprite.hpp>

using namespace geode::prelude;

class $modify(MIRobotSprite, GJRobotSprite) {
    void updateFrame(int frame) {
        GJRobotSprite::updateFrame(frame);

        MoreIconsAPI::removeUserObject(this);
    }
};
