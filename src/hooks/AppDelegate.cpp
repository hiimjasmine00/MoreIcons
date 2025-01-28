#include "../MoreIcons.hpp"
#include <Geode/modify/AppDelegate.hpp>

using namespace geode::prelude;

class $modify(MIAppDelegate, AppDelegate) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("AppDelegate::trySaveGame", -1);
    }

    void trySaveGame(bool closing) {
        auto mod = Mod::get();

        mod->setSavedValue<std::vector<std::string>>("icons", {});
        mod->setSavedValue<std::vector<std::string>>("ships", {});
        mod->setSavedValue<std::vector<std::string>>("balls", {});
        mod->setSavedValue<std::vector<std::string>>("ufos", {});
        mod->setSavedValue<std::vector<std::string>>("waves", {});
        mod->setSavedValue<std::vector<std::string>>("robots", {});
        mod->setSavedValue<std::vector<std::string>>("spiders", {});
        mod->setSavedValue<std::vector<std::string>>("swings", {});
        mod->setSavedValue<std::vector<std::string>>("jetpacks", {});
        mod->setSavedValue<std::vector<std::string>>("trails", {});

        AppDelegate::trySaveGame(closing);

        mod->setSavedValue("icons", MoreIconsAPI::ICONS);
        mod->setSavedValue("ships", MoreIconsAPI::SHIPS);
        mod->setSavedValue("balls", MoreIconsAPI::BALLS);
        mod->setSavedValue("ufos", MoreIconsAPI::UFOS);
        mod->setSavedValue("waves", MoreIconsAPI::WAVES);
        mod->setSavedValue("robots", MoreIconsAPI::ROBOTS);
        mod->setSavedValue("spiders", MoreIconsAPI::SPIDERS);
        mod->setSavedValue("swings", MoreIconsAPI::SWINGS);
        mod->setSavedValue("jetpacks", MoreIconsAPI::JETPACKS);
        mod->setSavedValue("trails", MoreIconsAPI::TRAILS);
    }
};
