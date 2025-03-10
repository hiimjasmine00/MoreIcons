#include "../MoreIcons.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(MIGameManager, GameManager) {
    void reloadAllStep2() {
        GameManager::reloadAllStep2();

        if (!m_unkBool1) return;

        MoreIcons::saveTrails();
        MoreIconsAPI::ICONS.clear();
        MoreIconsAPI::LOADED_ICONS.clear();
        MoreIcons::LOGS.clear();
        MoreIcons::HIGHEST_SEVERITY = Severity::Debug;
        MoreIcons::DEBUG_LOGS = Mod::get()->getSettingValue<bool>("debug-logs");
        MoreIcons::TRADITIONAL_PACKS = Mod::get()->getSettingValue<bool>("traditional-packs");
    }

    gd::string sheetNameForIcon(int id, int type) {
        auto ret = GameManager::sheetNameForIcon(id, type);
        if (ret.empty() || !MoreIcons::TRADITIONAL_PACKS) return ret;
        return MoreIcons::vanillaTexturePath(ret, false);
    }

    CCTexture2D* loadIcon(int id, int type, int requestID) {
        auto iconExists = m_loadIcon[keyForIcon(id, type)] > 0;
        auto ret = GameManager::loadIcon(id, type, requestID);
        if (!ret || iconExists || !MoreIcons::TRADITIONAL_PACKS) return ret;

        std::string sheetName = GameManager::sheetNameForIcon(id, type);
        if (sheetName.empty()) return ret;

        auto dict = CCDictionary::createWithContentsOfFile(fmt::format("{}.plist", sheetName).c_str());
        if (!dict) return ret;

        auto frames = static_cast<CCDictionary*>(dict->objectForKey("frames"));
        if (!frames) return ret;

        auto sfc = CCSpriteFrameCache::get();
        for (auto [frameName, _] : CCDictionaryExt<std::string, CCDictionary*>(frames)) {
            if (auto frame = sfc->spriteFrameByName(frameName.c_str())) frame->setTexture(ret);
        }

        return ret;
    }
};
