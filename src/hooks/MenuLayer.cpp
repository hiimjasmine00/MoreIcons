#include "../utils/Get.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <jasmine/hook.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

class $modify(MIMenuLayer, MenuLayer) {
    static void onModify(ModifyBase<ModifyDerive<MIMenuLayer, MenuLayer>>& self) {
        if (auto hook = jasmine::hook::get(self.m_hooks, "MenuLayer::init", false)) {
            if (auto iconProfile = Loader::get()->getInstalledMod("capeling.icon_profile")) {
                if (iconProfile->isLoaded()) {
                    hook->setAutoEnable(true);
                    ModifyBase<ModifyDerive<MIMenuLayer, MenuLayer>>::setHookPriorityAfterPost(hook, iconProfile);
                }
                else if (iconProfile->shouldLoad()) {
                    ModStateEvent(ModEventType::Loaded, iconProfile).listen([hook, iconProfile] {
                        ModifyBase<ModifyDerive<MIMenuLayer, MenuLayer>>::setHookPriorityAfterPost(hook, iconProfile);
                        jasmine::hook::toggle(hook, true);
                    }).leak();
                }
            }
        }
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto profileMenu = getChildByID("profile-menu");
        if (!profileMenu) return true;

        auto profileButton = static_cast<CCMenuItemSprite*>(profileMenu->getChildByID("profile-button"));
        if (!profileButton) return true;

        more_icons::updateSimplePlayer(profileButton->getNormalImage()->getChildByType<SimplePlayer>(0),
            Get::GameManager()->m_playerIconType, false);

        return true;
    }
};
