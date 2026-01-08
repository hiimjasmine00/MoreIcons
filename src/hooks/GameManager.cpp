#include "../utils/Get.hpp"
#include "../utils/Icons.hpp"
#include "../utils/Log.hpp"
#include <Geode/modify/GameManager.hpp>
#include <jasmine/hook.hpp>
#include <jasmine/setting.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

class $modify(MIGameManager, GameManager) {
    inline static Hook* sheetHook = nullptr;

    static void onModify(ModifyBase<ModifyDerive<MIGameManager, GameManager>>& self) {
        (void)self.setHookPriority("GameManager::loadIcon", Priority::Replace);
        sheetHook = jasmine::hook::get(self.m_hooks, "GameManager::sheetNameForIcon", jasmine::setting::getValue<bool>("traditional-packs"));
    }

    void reloadAllStep2() {
        GameManager::reloadAllStep2();

        if (!m_reloadTextures) return;

        Get::fileUtils = nullptr;
        Get::spriteFrameCache = nullptr;
        Get::textureCache = nullptr;
        more_icons::clearAllIcons();
        Icons::requestedIcons.clear();
        Icons::loadedIcons.clear();
        Log::logs.clear();
        Icons::loadSettings();
        jasmine::hook::toggle(sheetHook, Icons::traditionalPacks);
    }

    gd::string sheetNameForIcon(int id, int type) {
        auto ret = GameManager::sheetNameForIcon(id, type);
        if (ret.empty() || !Icons::traditionalPacks || id < 1) return ret;
        return Icons::vanillaTexturePath(ret, false);
    }

    CCTexture2D* loadIcon(int id, int type, int requestID) {
        std::string sheetName = GameManager::sheetNameForIcon(id, type);
        if (sheetName.empty()) return nullptr;

        CCTexture2D* texture = nullptr;
        auto iconKey = keyForIcon(id, type);

        auto pngName = fmt::format("{}.png", sheetName);
        auto textureCache = Get::TextureCache();
        if (m_iconLoadCounts[iconKey] < 1) {
            texture = textureCache->addImage(pngName.c_str(), false);
            Get::SpriteFrameCache()->addSpriteFramesWithFile(fmt::format("{}.plist", sheetName).c_str(), texture);
        }
        else texture = textureCache->textureForKey(pngName.c_str());

        auto loadedIcon = m_iconRequests[requestID][type];
        if (loadedIcon != id) {
            m_iconLoadCounts[iconKey]++;
            if (loadedIcon > 0) unloadIcon(loadedIcon, type, requestID);
            m_iconRequests[requestID][type] = id;
        }

        if (auto foundRequests = Icons::requestedIcons.find(requestID); foundRequests != Icons::requestedIcons.end()) {
            auto& iconRequests = foundRequests->second;
            if (auto found = iconRequests.find((IconType)type); found != iconRequests.end()) {
                more_icons::unloadIcon(found->second, requestID);
            }
        }

        return texture;
    }

    void unloadIcons(int requestID) {
        GameManager::unloadIcons(requestID);
        more_icons::unloadIcons(requestID);
    }
};
