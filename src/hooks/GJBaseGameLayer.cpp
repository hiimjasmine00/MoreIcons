#include "../MoreIcons.hpp"
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <jasmine/hook.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

class $modify(MIBaseGameLayer, GJBaseGameLayer) {
    static void onModify(ModifyBase<ModifyDerive<MIBaseGameLayer, GJBaseGameLayer>>& self) {
        if (auto hook = jasmine::hook::get(self.m_hooks, "GJBaseGameLayer::resetPlayer", false)) {
            if (auto separateDualIcons = Loader::get()->getInstalledMod("weebify.separate_dual_icons")) {
                if (separateDualIcons->isLoaded()) {
                    hook->setAutoEnable(true);
                    ModifyBase<ModifyDerive<MIBaseGameLayer, GJBaseGameLayer>>::setHookPriorityAfterPost(hook, separateDualIcons);
                }
                else if (separateDualIcons->shouldLoad()) {
                    ModStateEvent(ModEventType::Loaded, separateDualIcons).listen([hook, separateDualIcons] {
                        ModifyBase<ModifyDerive<MIBaseGameLayer, GJBaseGameLayer>>::setHookPriorityAfterPost(hook, separateDualIcons);
                        jasmine::hook::toggle(hook, true);
                    }).leak();
                }
            }
        }
    }

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
    void resetPlayer() {
        GJBaseGameLayer::resetPlayer();

        MoreIcons::setupCustomStreak(m_player1);
        MoreIcons::setupCustomStreak(m_player2);
    }
};
