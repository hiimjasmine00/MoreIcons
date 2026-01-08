#include "../utils/Get.hpp"
#include "../utils/Icons.hpp"
#include "../utils/Log.hpp"
#include <Geode/modify/GameManager.hpp>
#include <jasmine/hook.hpp>
#include <jasmine/setting.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

class $modify(MIGameManager, GameManager) {
    static void onModify(ModifyBase<ModifyDerive<MIGameManager, GameManager>>& self) {
        (void)self.setHookPriority("GameManager::loadIcon", Priority::Replace);
        if (auto hook = jasmine::hook::get(self.m_hooks, "GameManager::sheetNameForIcon", false)) {
            if (auto globed = Loader::get()->getInstalledMod("dankmeme.globed2")) {
                if (globed->isEnabled()) {
                    hook->setAutoEnable(jasmine::setting::getValue<bool>("traditional-packs"));
                    Icons::hooks.push_back(hook);
                }
                else if (globed->shouldLoad()) {
                    new EventListener([hook](ModStateEvent* e) {
                        jasmine::hook::toggle(hook, Icons::traditionalPacks);
                        Icons::hooks.push_back(hook);
                    }, ModStateFilter(globed, ModEventType::Loaded));
                }
            }
        }
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
        for (auto hook : Icons::hooks) {
            jasmine::hook::toggle(hook, Icons::traditionalPacks);
        }
    }

    gd::string sheetNameForIcon(int id, int type) {
        auto ret = GameManager::sheetNameForIcon(id, type);
        if (ret.empty() || id < 1) return ret;
        return Get::FileUtils()->fullPathForFilename(ret.c_str(), false);
    }

    CCTexture2D* loadIcon(int id, int type, int requestID) {
        auto texture = GameManager::loadIcon(id, type, requestID);
        if (!texture) return nullptr;

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
