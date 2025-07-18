#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(MIGameManager, GameManager) {
    inline static Hook* sheetHook = nullptr;

    static void onModify(ModifyBase<ModifyDerive<MIGameManager, GameManager>>& self) {
        (void)self.setHookPriority("GameManager::loadIcon", 999999999);
        (void)self.getHook("GameManager::sheetNameForIcon").inspect([](Hook* hook) {
            hook->setAutoEnable(Mod::get()->getSettingValue<bool>("traditional-packs"));
            sheetHook = hook;
        }).inspectErr([](const std::string& err) { log::error("Failed to get GameManager::sheetNameForIcon hook: {}", err); });
    }

    void reloadAllStep2() {
        GameManager::reloadAllStep2();

        if (!m_reloadTextures) return;

        MoreIcons::saveTrails();
        MoreIconsAPI::icons.clear();
        MoreIconsAPI::iconSpans.clear();
        MoreIconsAPI::requestedIcons.clear();
        MoreIconsAPI::loadedIcons.clear();
        MoreIcons::logs.clear();
        MoreIcons::severity = Severity::Debug;
        MoreIcons::severities = {
            { IconType::Cube, Severity::Debug },
            { IconType::Ship, Severity::Debug },
            { IconType::Ball, Severity::Debug },
            { IconType::Ufo, Severity::Debug },
            { IconType::Wave, Severity::Debug },
            { IconType::Robot, Severity::Debug },
            { IconType::Spider, Severity::Debug },
            { IconType::Swing, Severity::Debug },
            { IconType::Jetpack, Severity::Debug },
            { IconType::Special, Severity::Debug }
        };
        MoreIcons::loadSettings();

        if (sheetHook) (void)(MoreIcons::traditionalPacks ? sheetHook->enable().inspectErr([](const std::string& err) {
            log::error("Failed to enable GameManager::sheetNameForIcon hook: {}", err);
        }) : sheetHook->disable().inspectErr([](const std::string& err) {
            log::error("Failed to disable GameManager::sheetNameForIcon hook: {}", err);
        }));
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

        if (MoreIconsAPI::requestedIcons.contains(requestID)) {
            auto iconType = (IconType)type;
            auto& icons = MoreIconsAPI::requestedIcons[requestID];
            if (icons.contains(iconType)) MoreIconsAPI::unloadIcon(icons[iconType], iconType, requestID);
        }

        return texture;
    }

    void unloadIcons(int requestID) {
        GameManager::unloadIcons(requestID);
        MoreIconsAPI::unloadIcons(requestID);
    }
};
