#include "../MoreIcons.hpp"
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

class $modify(MIBaseGameLayer, GJBaseGameLayer) {
    void playExitDualEffect(PlayerObject* object) {
        GJBaseGameLayer::playExitDualEffect(object);

        if (auto player = findFirstChildRecursive<SimplePlayer>(this, [](SimplePlayer* node) {
            return node->getZOrder() == 100;
        })) {
            if (!m_player1 || m_player1 == object) {
                more_icons::updateSimplePlayer(player, more_icons::getIconType(object), false);
            }
            else if (!m_player2 || m_player2 == object) {
                more_icons::updateSimplePlayer(player, more_icons::getIconType(object), true);
            }
        }
    }
    #ifdef GEODE_IS_WINDOWS
    void resetLevelVariables() {
        auto blend = m_gameState.m_playerStreakBlend;
        GJBaseGameLayer::resetLevelVariables();

        if (blend) return;

        if (auto info = more_icons::activeIcon(IconType::Special, false)) MoreIcons::blendStreak(m_player1->m_regularTrail, info);
        if (auto info = more_icons::activeIcon(IconType::Special, true)) MoreIcons::blendStreak(m_player2->m_regularTrail, info);
    }
    #endif
};
