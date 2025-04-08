#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(MIGameManager, GameManager) {
    inline static Hook* sheetHook = nullptr;

    static void onModify(ModifyBase<ModifyDerive<MIGameManager, GameManager>>& self) {
        (void)self.getHook("GameManager::sheetNameForIcon").map([](Hook* hook) {
            hook->setAutoEnable(Mod::get()->getSettingValue<bool>("traditional-packs"));
            return sheetHook = hook;
        }).mapErr([](const std::string& err) {
            return log::error("Failed to get GameManager::sheetNameForIcon hook: {}", err), err;
        });
    }

    void reloadAllStep2() {
        GameManager::reloadAllStep2();

        if (!m_reloadTextures) return;

        MoreIcons::saveTrails();
        MoreIconsAPI::icons.clear();
        MoreIconsAPI::iconIndices.clear();
        MoreIconsAPI::requestedIcons.clear();
        MoreIconsAPI::loadedIcons.clear();
        MoreIcons::logs.clear();
        MoreIcons::severity = Severity::Debug;

        auto mod = Mod::get();
        MoreIcons::debugLogs = mod->getSettingValue<bool>("debug-logs");
        MoreIcons::traditionalPacks = mod->getSettingValue<bool>("traditional-packs");

        auto hooks = mod->getHooks();

        if (sheetHook) (void)(MoreIcons::traditionalPacks ? sheetHook->enable().mapErr([](const std::string& err) {
            return log::error("Failed to enable GameManager::sheetNameForIcon hook: {}", err), err;
        }) : sheetHook->disable().mapErr([](const std::string& err) {
            return log::error("Failed to disable GameManager::sheetNameForIcon hook: {}", err), err;
        }));
    }

    gd::string sheetNameForIcon(int id, int type) {
        auto ret = GameManager::sheetNameForIcon(id, type);
        if (ret.empty() || !MoreIcons::traditionalPacks) return ret;
        return MoreIcons::vanillaTexturePath(ret, false);
    }

    CCTexture2D* loadIcon(int id, int type, int requestID) {
        auto iconKey = keyForIcon(id, type);
        auto iconExists = m_iconLoadCounts.contains(iconKey) && m_iconLoadCounts[iconKey] > 0;

        auto ret = GameManager::loadIcon(id, type, requestID);
        if (!ret) return ret;

        if (MoreIconsAPI::requestedIcons.contains(requestID)) {
            auto iconType = (IconType)type;
            if (auto& icons = MoreIconsAPI::requestedIcons[requestID]; icons.contains(iconType))
                MoreIconsAPI::unloadIcon(icons[iconType], iconType, requestID);
        }

        if (iconExists || !MoreIcons::traditionalPacks) return ret;

        auto sheetName = GameManager::sheetNameForIcon(id, type);
        if (sheetName.empty()) return ret;

        auto dict = CCDictionary::createWithContentsOfFileThreadSafe(fmt::format("{}.plist", GEODE_ANDROID(std::string)(sheetName)).c_str());
        if (!dict) return ret;

        auto frames = static_cast<CCDictionary*>(dict->objectForKey("frames"));
        if (!frames) return dict->release(), ret;

        auto sfc = CCSpriteFrameCache::get();
        for (auto [frameName, _] : CCDictionaryExt<std::string, CCDictionary*>(frames)) {
            if (auto frame = sfc->spriteFrameByName(frameName.c_str())) frame->setTexture(ret);
        }

        return dict->release(), ret;
    }

    void unloadIcons(int requestID) {
        GameManager::unloadIcons(requestID);
        MoreIconsAPI::unloadIcons(requestID);
    }
};
