#include "../MoreIcons.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(MIGameManager, GameManager) {
    void reloadAllStep2() {
        GameManager::reloadAllStep2();

        if (!m_unkBool1) return;

        MoreIconsAPI::ICONS.clear();
        MoreIcons::ICON_INFO.clear();
        MoreIconsAPI::SHIPS.clear();
        MoreIcons::SHIP_INFO.clear();
        MoreIconsAPI::BALLS.clear();
        MoreIcons::BALL_INFO.clear();
        MoreIconsAPI::UFOS.clear();
        MoreIcons::UFO_INFO.clear();
        MoreIconsAPI::WAVES.clear();
        MoreIcons::WAVE_INFO.clear();
        MoreIconsAPI::ROBOTS.clear();
        MoreIcons::ROBOT_INFO.clear();
        MoreIconsAPI::SPIDERS.clear();
        MoreIcons::SPIDER_INFO.clear();
        MoreIconsAPI::SWINGS.clear();
        MoreIcons::SWING_INFO.clear();
        MoreIconsAPI::JETPACKS.clear();
        MoreIcons::JETPACK_INFO.clear();
        MoreIconsAPI::TRAILS.clear();
        MoreIcons::saveTrails();
        MoreIcons::TRAIL_INFO.clear();
        MoreIcons::LOGS.clear();
        MoreIcons::HIGHEST_SEVERITY = LogType::Info;
    }
};
