#include "../MoreIcons.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(MIMenuLayer, MenuLayer) {
    static void onModify(auto& self) {
        if (auto initHookRes = self.getHook("MenuLayer::init")) {
            auto initHook = initHookRes.unwrap();
            if (auto iconProfile = Loader::get()->getInstalledMod("capeling.icon_profile")) {
                if (iconProfile->shouldLoad()) return initHook->setPriority(-1);
            }

            queueInMainThread([initHook] {
                if (!initHook->disable()) log::error("Failed to disable MenuLayer::init hook");
            });
        }
        else log::error("Failed to find MenuLayer::init hook");
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto profileMenu = getChildByID("profile-menu");
        if (!profileMenu) return true;

        auto profileButton = static_cast<CCMenuItemSpriteExtra*>(profileMenu->getChildByID("profile-button"));
        if (!profileButton) return true;

        MoreIcons::changeSimplePlayer(profileButton->getNormalImage()->getChildByType<SimplePlayer>(0), GameManager::get()->m_playerIconType, false);

        return true;
    }
};
