#include "../api/MoreIconsAPI.hpp"
#include <Geode/modify/SimplePlayer.hpp>

using namespace geode::prelude;

class $modify(MISimplePlayer, SimplePlayer) {
    void updatePlayerFrame(int frame, IconType type) {
        SimplePlayer::updatePlayerFrame(frame, type);

        MoreIconsAPI::removeUserObject(this);
    }
};
