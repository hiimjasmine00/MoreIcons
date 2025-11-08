#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <jasmine/hook.hpp>

using namespace geode::prelude;

class $modify(MIMenuLayer, MenuLayer) {
    static void onModify(ModifyBase<ModifyDerive<MIMenuLayer, MenuLayer>>& self) {
        if (auto hook = jasmine::hook::get(self.m_hooks, "MenuLayer::init", false)) {
            if (auto iconProfile = Loader::get()->getInstalledMod("capeling.icon_profile")) {
                if (iconProfile->isEnabled()) {
                    hook->setAutoEnable(true);
                    afterPriority(hook, iconProfile);
                }
                else new EventListener([hook](ModStateEvent* e) {
                    afterPriority(hook, e->getMod());
                    jasmine::hook::toggle(hook, true);
                }, ModStateFilter(iconProfile, ModEventType::Loaded));
            }
        }
    }

    static void afterPriority(Hook* hook, Mod* mod) {
        auto address = hook->getAddress();
        auto modHooks = mod->getHooks();
        auto modHook = std::ranges::find_if(modHooks, [address](Hook* h) {
            return h->getAddress() == address;
        });
        if (modHook == modHooks.end()) return log::error("Failed to find MenuLayer::init hook in capeling.icon_profile");

        auto priority = (*modHook)->getPriority();
        if (hook->getPriority() >= priority) hook->setPriority(priority - 1);
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto profileMenu = getChildByID("profile-menu");
        if (!profileMenu) return true;

        auto profileButton = static_cast<CCMenuItemSprite*>(profileMenu->getChildByID("profile-button"));
        if (!profileButton) return true;

        MoreIconsAPI::updateSimplePlayer(profileButton->getNormalImage()->getChildByType<SimplePlayer>(0),
            MoreIconsAPI::getGameManager()->m_playerIconType, false);

        return true;
    }
};
