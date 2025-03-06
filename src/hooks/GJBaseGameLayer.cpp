#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(MIBaseGameLayer, GJBaseGameLayer) {
    void playExitDualEffect(PlayerObject* object) {
        GJBaseGameLayer::playExitDualEffect(object);

        if (auto player = findFirstChildRecursive<SimplePlayer>(this, [](SimplePlayer* node) { return node->getZOrder() == 100; })) {
            if (!m_player1 || m_player1 == object) MoreIconsAPI::updateSimplePlayer(player, MoreIconsAPI::getIconType(object), false);
            else if (!m_player2 || m_player2 == object) MoreIconsAPI::updateSimplePlayer(player, MoreIconsAPI::getIconType(object), true);
        }
    }
};
