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
        MoreIcons::GLOBED_ICONS_LOADED = false;
    }

    gd::string sheetNameForIcon(int id, int type) {
        auto ret = GameManager::sheetNameForIcon(id, type);
        if (ret.empty()) return ret;
        return MoreIcons::vanillaTexturePath(ret, false);
    }

    CCTexture2D* loadIcon(int id, int type, int requestID) {
        auto iconExists = m_loadIcon[keyForIcon(id, type)] > 0;
        auto ret = GameManager::loadIcon(id, type, requestID);
        if (!ret || iconExists) return ret;
        std::string sheetName = GameManager::sheetNameForIcon(id, type);
        if (sheetName.empty()) return ret;
        auto dict = CCContentManager::sharedManager()->addDict((sheetName + ".plist").c_str(), false);
        if (!dict) return ret;
        auto frames = static_cast<CCDictionary*>(dict->objectForKey("frames"));
        if (!frames) return ret;
        auto sfc = CCSpriteFrameCache::sharedSpriteFrameCache();
        for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(frames)) {
            sfc->spriteFrameByName(frameName.c_str())->setTexture(ret);
        }
        return ret;
    }
};
