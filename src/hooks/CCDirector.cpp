#include "../utils/Get.hpp"
#include <Geode/modify/CCDirector.hpp>

using namespace geode::prelude;

class $modify(MIDirector, CCDirector) {
    void purgeDirector() {
        Get::animationCache = nullptr;
        Get::director = nullptr;
        Get::fileUtils = nullptr;
        Get::spriteFrameCache = nullptr;
        Get::textureCache = nullptr;
        CCDirector::purgeDirector();
    }
};
