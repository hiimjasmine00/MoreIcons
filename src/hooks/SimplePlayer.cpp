#include "../utils/Icons.hpp"
#include <Geode/modify/SimplePlayer.hpp>

using namespace geode::prelude;

class $modify(MISimplePlayer, SimplePlayer) {
    void updatePlayerFrame(int frame, IconType type) {
        SimplePlayer::updatePlayerFrame(frame, type);

        Icons::setIcon(this, nullptr);
    }
};
