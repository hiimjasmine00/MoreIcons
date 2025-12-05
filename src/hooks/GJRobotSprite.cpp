#include <Geode/modify/GJRobotSprite.hpp>

using namespace geode::prelude;

class $modify(MIRobotSprite, GJRobotSprite) {
    void updateFrame(int frame) {
        GJRobotSprite::updateFrame(frame);

        setUserObject("name"_spr, nullptr);
    }
};
