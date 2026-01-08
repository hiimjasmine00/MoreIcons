#include "../utils/Icons.hpp"
#include <Geode/modify/GJRobotSprite.hpp>

using namespace geode::prelude;

class $modify(MIRobotSprite, GJRobotSprite) {
    void updateFrame(int frame) {
        GJRobotSprite::updateFrame(frame);

        Icons::setIcon(this, nullptr);
    }
};
