#include <Geode/modify/SimplePlayer.hpp>

using namespace geode::prelude;

class $modify(MISimplePlayer, SimplePlayer) {
    void updatePlayerFrame(int frame, IconType type) {
        SimplePlayer::updatePlayerFrame(frame, type);

        if (auto str = static_cast<CCString*>(getUserObject("name"_spr))) {
            str->m_sString.clear();
        }
    }
};
