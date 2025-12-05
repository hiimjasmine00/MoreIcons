#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <MoreIconsV2.hpp>

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

        if (auto info = more_icons::getIcon(IconType::Special, false)) {
            m_player1->m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, (uint32_t)(info->trailInfo.blend ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA) });
        }

        if (auto info = more_icons::getIcon(IconType::Special, true)) {
            m_player2->m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, (uint32_t)(info->trailInfo.blend ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA) });
        }
    }
    #endif
};
