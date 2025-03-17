#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/modify/GameManager.hpp>
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

class $modify(MIGameManager, GameManager) {
    struct Fields {
        inline static uintptr_t SHEET_ADDRESS = 0;
        inline static uintptr_t LOAD_ADDRESS = 0;
    };

    static void onModify(ModifyBase<ModifyDerive<MIGameManager, GameManager>>& self) {
        auto traditionalPacks = Mod::get()->getSettingValue<bool>("traditional-packs");

        auto sheetHook = self.getHook("GameManager::sheetNameForIcon").mapErr([](const std::string& err) {
            return log::error("Failed to get GameManager::sheetNameForIcon hook: {}", err), err;
        }).unwrapOr(nullptr);
        if (sheetHook) {
            Fields::SHEET_ADDRESS = sheetHook->getAddress();
            sheetHook->setAutoEnable(traditionalPacks);
        }

        auto loadHook = self.getHook("GameManager::loadIcon").mapErr([](const std::string& err) {
            return log::error("Failed to get GameManager::loadIcon hook: {}", err), err;
        }).unwrapOr(nullptr);
        if (loadHook) {
            Fields::LOAD_ADDRESS = loadHook->getAddress();
            loadHook->setAutoEnable(traditionalPacks);
        }
    }

    void reloadAllStep2() {
        GameManager::reloadAllStep2();

        if (!m_unkBool1) return;

        MoreIcons::saveTrails();
        MoreIconsAPI::ICONS.clear();
        MoreIconsAPI::LOADED_ICONS.clear();
        MoreIcons::LOGS.clear();
        MoreIcons::HIGHEST_SEVERITY = Severity::Debug;

        auto mod = Mod::get();
        MoreIcons::DEBUG_LOGS = mod->getSettingValue<bool>("debug-logs");
        MoreIcons::TRADITIONAL_PACKS = mod->getSettingValue<bool>("traditional-packs");

        auto hooks = mod->getHooks();

        if (auto sheetHook = ranges::find(hooks, [](Hook* hook) { return hook->getAddress() == Fields::SHEET_ADDRESS; }))
            (void)(MoreIcons::TRADITIONAL_PACKS ? sheetHook.value()->enable().mapErr([](const std::string& err) {
                return log::error("Failed to enable GameManager::sheetNameForIcon hook: {}", err), err;
            }) : sheetHook.value()->disable().mapErr([](const std::string& err) {
                return log::error("Failed to disable GameManager::sheetNameForIcon hook: {}", err), err;
            }));
        else log::error("Failed to find GameManager::sheetNameForIcon hook");

        if (auto loadHook = ranges::find(hooks, [](Hook* hook) { return hook->getAddress() == Fields::LOAD_ADDRESS; }))
            (void)(MoreIcons::TRADITIONAL_PACKS ? loadHook.value()->enable().mapErr([](const std::string& err) {
                return log::error("Failed to enable GameManager::loadIcon hook: {}", err), err;
            }) : loadHook.value()->disable().mapErr([](const std::string& err) {
                return log::error("Failed to disable GameManager::loadIcon hook: {}", err), err;
            }));
        else log::error("Failed to find GameManager::loadIcon hook");
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

        auto sheetName = GameManager::sheetNameForIcon(id, type);
        if (sheetName.empty()) return ret;

        auto dict = CCDictionary::createWithContentsOfFileThreadSafe(fmt::format("{}.plist", GEODE_ANDROID(std::string)(sheetName)).c_str());
        if (!dict) return ret;

        auto frames = static_cast<CCDictionary*>(dict->objectForKey("frames"));
        if (!frames) return ret;

        auto sfc = CCSpriteFrameCache::get();
        for (auto [frameName, _] : CCDictionaryExt<std::string, CCDictionary*>(frames)) {
            if (auto frame = sfc->spriteFrameByName(frameName.c_str())) frame->setTexture(ret);
        }

        dict->release();

        return ret;
    }
};
