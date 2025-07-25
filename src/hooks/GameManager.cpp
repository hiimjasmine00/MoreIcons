#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(MIGameManager, GameManager) {
    inline static Hook* sheetHook = nullptr;

    static void onModify(ModifyBase<ModifyDerive<MIGameManager, GameManager>>& self) {
        (void)self.setHookPriority("GameManager::loadIcon", 999999999);
        self.getHook("GameManager::sheetNameForIcon").inspect([](Hook* hook) {
            hook->setAutoEnable(Mod::get()->getSettingValue<bool>("traditional-packs"));
            sheetHook = hook;
        }).inspectErr([](const std::string& err) { log::error("Failed to get GameManager::sheetNameForIcon hook: {}", err); });
    }

    void reloadAllStep2() {
        GameManager::reloadAllStep2();

        if (!m_reloadTextures) return;

        MoreIcons::saveTrails();
        MoreIconsAPI::icons[IconType::Cube].clear();
        MoreIconsAPI::icons[IconType::Ship].clear();
        MoreIconsAPI::icons[IconType::Ball].clear();
        MoreIconsAPI::icons[IconType::Ufo].clear();
        MoreIconsAPI::icons[IconType::Wave].clear();
        MoreIconsAPI::icons[IconType::Robot].clear();
        MoreIconsAPI::icons[IconType::Spider].clear();
        MoreIconsAPI::icons[IconType::Swing].clear();
        MoreIconsAPI::icons[IconType::Jetpack].clear();
        MoreIconsAPI::icons[IconType::Special].clear();
        MoreIconsAPI::requestedIcons.clear();
        MoreIconsAPI::loadedIcons.clear();
        MoreIcons::logs.clear();
        MoreIcons::severity = Severity::Debug;
        MoreIcons::severities[IconType::Cube] = Severity::Debug;
        MoreIcons::severities[IconType::Ship] = Severity::Debug;
        MoreIcons::severities[IconType::Ball] = Severity::Debug;
        MoreIcons::severities[IconType::Ufo] = Severity::Debug;
        MoreIcons::severities[IconType::Wave] = Severity::Debug;
        MoreIcons::severities[IconType::Robot] = Severity::Debug;
        MoreIcons::severities[IconType::Spider] = Severity::Debug;
        MoreIcons::severities[IconType::Swing] = Severity::Debug;
        MoreIcons::severities[IconType::Jetpack] = Severity::Debug;
        MoreIcons::severities[IconType::Special] = Severity::Debug;
        MoreIcons::loadSettings();

        if (sheetHook) {
            if (MoreIcons::traditionalPacks) sheetHook->enable().inspectErr([](const std::string& err) {
                log::error("Failed to enable GameManager::sheetNameForIcon hook: {}", err);
            });
            else sheetHook->disable().inspectErr([](const std::string& err) {
                log::error("Failed to disable GameManager::sheetNameForIcon hook: {}", err);
            });
        }
    }

    gd::string sheetNameForIcon(int id, int type) {
        auto ret = GameManager::sheetNameForIcon(id, type);
        if (ret.empty() || !MoreIcons::traditionalPacks || id < 1) return ret;
        return MoreIcons::vanillaTexturePath(ret, false);
    }

    CCTexture2D* loadIcon(int id, int type, int requestID) {
        std::string sheetName = GameManager::sheetNameForIcon(id, type);
        if (sheetName.empty()) return nullptr;

        CCTexture2D* texture = nullptr;
        auto iconKey = keyForIcon(id, type);

        auto pngName = fmt::format("{}.png", sheetName);
        auto textureCache = CCTextureCache::get();
        if (m_iconLoadCounts[iconKey] < 1) {
            texture = textureCache->addImage(pngName.c_str(), false);
            CCSpriteFrameCache::get()->addSpriteFramesWithFile(fmt::format("{}.plist", sheetName).c_str(), texture);
        }
        else texture = textureCache->textureForKey(pngName.c_str());

        auto loadedIcon = m_iconRequests[requestID][type];
        if (loadedIcon != id) {
            m_iconLoadCounts[iconKey]++;
            if (loadedIcon > 0) unloadIcon(loadedIcon, type, requestID);
            m_iconRequests[requestID][type] = id;
        }

        if (auto foundRequests = MoreIconsAPI::requestedIcons.find(requestID); foundRequests != MoreIconsAPI::requestedIcons.end()) {
            auto iconType = (IconType)type;
            auto& iconRequests = foundRequests->second;
            if (auto found = iconRequests.find(iconType); found != iconRequests.end()) MoreIconsAPI::unloadIcon(found->second, iconType, requestID);
        }

        return texture;
    }

    void unloadIcons(int requestID) {
        GameManager::unloadIcons(requestID);
        MoreIconsAPI::unloadIcons(requestID);
    }
};
