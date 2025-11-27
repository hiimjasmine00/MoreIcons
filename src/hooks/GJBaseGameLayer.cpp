#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/CCCircleWave.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(MIBaseGameLayer, GJBaseGameLayer) {
    static void onModify(ModifyBase<ModifyDerive<MIBaseGameLayer, GJBaseGameLayer>>& self) {
        (void)self.setHookPriority("GJBaseGameLayer::playExitDualEffect", Priority::Replace);
    }

    void playExitDualEffect(PlayerObject* player) {
        auto player1 = !m_player1 || m_player1 == player;
        auto player2 = !m_player2 || m_player2 == player;
        if (!player1 && !player2) return GJBaseGameLayer::playExitDualEffect(player);

        auto iconType = MoreIconsAPI::getIconType(player);

        std::string activeIcon;
        if (player1) activeIcon = MoreIconsAPI::activeIcon(iconType, false);
        else if (player2) activeIcon = MoreIconsAPI::activeIcon(iconType, true);

        auto simplePlayer = SimplePlayer::create(MoreIcons::vanillaIcon(iconType, false));
        if (activeIcon.empty()) {
            if (player1) simplePlayer->updatePlayerFrame(MoreIcons::vanillaIcon(iconType, false), iconType);
            else if (player2) simplePlayer->updatePlayerFrame(MoreIcons::vanillaIcon(iconType, true), iconType);
        }
        else MoreIconsAPI::updateSimplePlayer(simplePlayer, activeIcon, iconType);

        auto cameraZoom = m_gameState.m_cameraZoom;
        simplePlayer->setScaleX(player->getScaleX() * cameraZoom);
        simplePlayer->setScaleY(player->getScaleY() * cameraZoom);
        auto position = player->getPosition();
        auto rotation = player->getRotation();
        auto factor = 1;
        if (m_gameState.m_unkBool10) {
            factor = -1;
            rotation = -rotation;
            simplePlayer->setScaleX(-simplePlayer->getScaleX());
            position.x += 150.0f;
        }
        addChild(simplePlayer, 100);

        position -= m_gameState.m_cameraPosition;
        simplePlayer->setColor(player->getColor());
        simplePlayer->setSecondColor(player->getSecondColor());
        simplePlayer->updateColors();
        simplePlayer->setPosition(position * cameraZoom);
        simplePlayer->setRotation(rotation);
        simplePlayer->runAction(CCFadeOut::create(0.4f));
        simplePlayer->runAction(CCMoveTo::create(0.4f, simplePlayer->getPosition() + CCPoint { -124.632034f * factor, 0.0f }));
        simplePlayer->runAction(CCSequence::create(
            CCScaleTo::create(0.4f, simplePlayer->getScaleX() > 0.0f ? 0.1f : -0.1f, simplePlayer->getScaleY() > 0.0f ? 0.1f : -0.1f),
            CCCallFunc::create(simplePlayer, callfunc_selector(CCNode::removeMeAndCleanup)),
            nullptr
        ));
        simplePlayer->runAction(CCRotateBy::create(0.4f, 180.0f));

        auto circleWave = CCCircleWave::create(10.0f, 30.0f, 0.4f, false);
        circleWave->m_color = player->m_playerColor1;
        circleWave->setPosition(position);
        addChild(circleWave, 0);
        circleWave->m_circleMode = CircleMode::Outline;
        circleWave->followObject(player, false);
    }
    #ifdef GEODE_IS_WINDOWS
    void resetLevelVariables() {
        auto blend = m_gameState.m_playerStreakBlend;
        GJBaseGameLayer::resetLevelVariables();

        if (blend) return;

        if (auto info = MoreIconsAPI::getIcon(IconType::Special, false)) {
            m_player1->m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, (uint32_t)(info->trailInfo.blend ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA) });
        }

        if (auto info = MoreIconsAPI::getIcon(IconType::Special, true)) {
            m_player2->m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, (uint32_t)(info->trailInfo.blend ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA) });
        }
    }
    #endif
};
