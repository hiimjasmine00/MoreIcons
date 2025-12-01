#include "../api/MoreIconsAPI.hpp"
#include <Geode/modify/CCDirector.hpp>

using namespace geode::prelude;

class $modify(MIDirector, CCDirector) {
    void purgeDirector() {
        MoreIconsAPI::directorPurged();
        CCDirector::purgeDirector();
    }
};
