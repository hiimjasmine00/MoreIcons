#include "../MoreIcons.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(MIGameManager, GameManager) {
    void reloadAllStep2() {
        GameManager::reloadAllStep2();

        if (m_unkBool1) {
            MoreIconsAPI::ICONS.clear();
            MoreIcons::ICON_INFO.clear();
            Mod::get()->setSavedValue<std::vector<std::string>>("icons", {});
            MoreIconsAPI::SHIPS.clear();
            MoreIcons::SHIP_INFO.clear();
            Mod::get()->setSavedValue<std::vector<std::string>>("ships", {});
            MoreIconsAPI::BALLS.clear();
            MoreIcons::BALL_INFO.clear();
            Mod::get()->setSavedValue<std::vector<std::string>>("balls", {});
            MoreIconsAPI::UFOS.clear();
            MoreIcons::UFO_INFO.clear();
            Mod::get()->setSavedValue<std::vector<std::string>>("ufos", {});
            MoreIconsAPI::WAVES.clear();
            MoreIcons::WAVE_INFO.clear();
            Mod::get()->setSavedValue<std::vector<std::string>>("waves", {});
            MoreIconsAPI::ROBOTS.clear();
            MoreIcons::ROBOT_INFO.clear();
            Mod::get()->setSavedValue<std::vector<std::string>>("robots", {});
            MoreIconsAPI::SPIDERS.clear();
            MoreIcons::SPIDER_INFO.clear();
            Mod::get()->setSavedValue<std::vector<std::string>>("spiders", {});
            MoreIconsAPI::SWINGS.clear();
            MoreIcons::SWING_INFO.clear();
            Mod::get()->setSavedValue<std::vector<std::string>>("swings", {});
            MoreIconsAPI::JETPACKS.clear();
            MoreIcons::JETPACK_INFO.clear();
            Mod::get()->setSavedValue<std::vector<std::string>>("jetpacks", {});
            MoreIconsAPI::TRAILS.clear();
            MoreIcons::saveTrails();
            MoreIcons::TRAIL_INFO.clear();
            Mod::get()->setSavedValue<std::vector<std::string>>("trails", {});
            MoreIcons::LOGS.clear();
            MoreIcons::HIGHEST_SEVERITY = LogType::Info;
        }
    }
};
