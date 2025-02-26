#include "../MoreIcons.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

class $modify(MIMenuLayer, MenuLayer) {
    static void onModify(ModifyBase<ModifyDerive<MIMenuLayer, MenuLayer>>& self) {
        auto initRes = self.getHook("MenuLayer::init");
        if (initRes.isErr()) return log::error("Failed to get MenuLayer::init hook: {}", initRes.unwrapErr());

        auto initHook = initRes.unwrap();
        initHook->setAutoEnable(false);
        if (auto iconProfile = Loader::get()->getInstalledMod("capeling.icon_profile")) {
            auto func = [iconProfile, initHook](ModStateEvent*) {
                auto enableRes = initHook->enable();
                if (enableRes.isErr()) return log::error("Failed to enable MenuLayer::init hook: {}", enableRes.unwrapErr());
                auto address = initHook->getAddress();
                auto hookOpt = ranges::find(iconProfile->getHooks(), [address](Hook* hook) { return hook->getAddress() == address; });
                if (!hookOpt.has_value()) return log::error("Failed to find MenuLayer::init hook in capeling.icon_profile");
                auto hookPriority = hookOpt.value()->getPriority();
                if (initHook->getPriority() >= hookPriority) initHook->setPriority(hookPriority - 1);
            };
            if (iconProfile->isEnabled()) func(nullptr);
            else new EventListener(func, ModStateFilter(iconProfile, ModEventType::Loaded));
        }
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        MoreIcons::GLOBED_ICONS_LOADED = true;

        auto profileMenu = getChildByID("profile-menu");
        if (!profileMenu) return true;

        auto profileButton = static_cast<CCMenuItemSpriteExtra*>(profileMenu->getChildByID("profile-button"));
        if (!profileButton) return true;

        MoreIcons::changeSimplePlayer(profileButton->getNormalImage()->getChildByType<SimplePlayer>(0), GameManager::get()->m_playerIconType, false);

        return true;
    }
};
