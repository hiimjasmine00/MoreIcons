#include "../MoreIcons.hpp"
#include "../utils/Get.hpp"
#include <Geode/modify/GameManager.hpp>
#include <jasmine/hook.hpp>
#include <jasmine/setting.hpp>
#include <MoreIcons.hpp>
#include <ranges>

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
        Get::objectManager = nullptr;
        std::ranges::for_each(std::views::values(MoreIcons::icons), &std::vector<IconInfo>::clear);
        MoreIcons::requestedIcons.clear();
        MoreIcons::loadedIcons.clear();
        MoreIcons::logs.clear();
        MoreIcons::severity = Severity::Debug;
        for (auto& severity : std::views::values(MoreIcons::severities)) {
            severity = Severity::Debug;
        }
        MoreIcons::loadSettings();
        jasmine::hook::toggle(sheetHook, MoreIcons::traditionalPacks);
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

        if (auto foundRequests = MoreIcons::requestedIcons.find(requestID); foundRequests != MoreIcons::requestedIcons.end()) {
            auto iconType = (IconType)type;
            auto& iconRequests = foundRequests->second;
            if (auto found = iconRequests.find(iconType); found != iconRequests.end()) {
                more_icons::unloadIcon(found->second, iconType, requestID);
            }
        }

        return texture;
    }

    void unloadIcons(int requestID) {
        GameManager::unloadIcons(requestID);
        more_icons::unloadIcons(requestID);
    }
};
