#include "../MoreIcons.hpp"

using namespace geode::prelude;

#include <Geode/modify/GJBaseGameLayer.hpp>
class $modify(MIBaseGameLayer, GJBaseGameLayer) {
    void playExitDualEffect(PlayerObject* object) {
        GJBaseGameLayer::playExitDualEffect(object);

        if (auto player = findFirstChildRecursive<SimplePlayer>(this, [](SimplePlayer* node) { return node->getZOrder() == 100; })) {
            if (!m_player1 || m_player1 == object)
                MoreIcons::changeSimplePlayer(player, MoreIconsAPI::getIconType(object), false);
            else if (!m_player2 || m_player2 == object)
                MoreIcons::changeSimplePlayer(player, MoreIconsAPI::getIconType(object), true);
        }
    }
};
