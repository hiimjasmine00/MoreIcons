#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

class $modify(MIMenuLayer, MenuLayer) {
    static void onModify(ModifyBase<ModifyDerive<MIMenuLayer, MenuLayer>>& self) {
        auto initHook = self.getHook("MenuLayer::init").mapErr([](const std::string& err) {
            return log::error("Failed to get MenuLayer::init hook: {}", err), err;
        }).unwrapOr(nullptr);
        if (!initHook) return;

        initHook->setAutoEnable(false);

        if (auto iconProfile = Loader::get()->getInstalledMod("capeling.icon_profile")) {
            auto func = [iconProfile, initHook](ModStateEvent* event) {
                if (!event) initHook->setAutoEnable(true);

                auto address = initHook->getAddress();
                auto modHook = ranges::find(iconProfile->getHooks(), [address](Hook* hook) { return hook->getAddress() == address; });
                if (!modHook.has_value()) return log::error("Failed to find MenuLayer::init hook in capeling.icon_profile");

                auto hookPriority = modHook.value()->getPriority();
                if (initHook->getPriority() >= hookPriority) initHook->setPriority(hookPriority - 1);

                if (event) (void)initHook->enable().mapErr([](const std::string& err) {
                    return log::error("Failed to enable MenuLayer::init hook: {}", err), err;
                });
            };

            if (iconProfile->isEnabled()) func(nullptr);
            else new EventListener(func, ModStateFilter(iconProfile, ModEventType::Loaded));
        }
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto profileMenu = getChildByID("profile-menu");
        if (!profileMenu) return true;

        auto profileButton = static_cast<CCMenuItemSprite*>(profileMenu->getChildByID("profile-button"));
        if (!profileButton) return true;

        MoreIconsAPI::updateSimplePlayer(profileButton->getNormalImage()->getChildByType<SimplePlayer>(0), GameManager::get()->m_playerIconType, false);

        return true;
    }
};
