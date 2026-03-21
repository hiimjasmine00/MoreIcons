#include "../utils/Filesystem.hpp"
#include "../utils/Get.hpp"
#include "../utils/Icons.hpp"
#include "../utils/Log.hpp"
#include <Geode/loader/Dirs.hpp>
#include <Geode/modify/GameManager.hpp>
#include <jasmine/hook.hpp>
#include <jasmine/setting.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

std::string addSuffix(std::string str, std::string_view suffix) {
    if (!str.contains(suffix)) {
        auto dotPos = str.rfind('.');
        auto dashPos = str.rfind('-');
        if (dotPos != std::string::npos && (dashPos == std::string::npos || dotPos > dashPos)) {
            str.insert(dotPos, suffix);
        }
    }
    return std::move(str);
}

bool isFileExist(const gd::string& path) {
    #ifdef GEODE_IS_WINDOWS
    auto attrs = GetFileAttributesA(path.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0);
    #else
    return Get::fileUtils->isFileExist(path);
    #endif
}

class $modify(MIGameManager, GameManager) {
    static void onModify(ModifyBase<ModifyDerive<MIGameManager, GameManager>>& self) {
        if (auto hook = jasmine::hook::get(self.m_hooks, "GameManager::sheetNameForIcon", false)) {
            if (auto globed = Loader::get()->getInstalledMod("dankmeme.globed2")) {
                if (globed->isLoaded()) {
                    hook->setAutoEnable(jasmine::setting::getValue<bool>("traditional-packs"));
                    Icons::hooks.push_back(hook);
                }
                else if (globed->shouldLoad()) {
                    ModStateEvent(ModEventType::Loaded, globed).listen([hook] {
                        jasmine::hook::toggle(hook, Icons::traditionalPacks);
                        Icons::hooks.push_back(hook);
                    }).leak();
                }
            }
        }
    }

    void reloadAllStep2() {
        GameManager::reloadAllStep2();

        if (!m_reloadTextures) return;

        Get::fileUtils = CCFileUtils::sharedFileUtils();
        Get::spriteFrameCache = nullptr;
        Get::textureCache = nullptr;
        more_icons::clearAllIcons();
        Icons::requestedIcons.clear();
        Icons::loadedIcons.clear();
        Icons::iconWrappers.clear();
        Icons::loadingFinished = false;
        Log::logs.clear();
        Icons::loadSettings();
        for (auto hook : Icons::hooks) {
            jasmine::hook::toggle(hook, Icons::traditionalPacks);
        }
    }

    gd::string sheetNameForIcon(int id, int type) {
        auto ret = GameManager::sheetNameForIcon(id, type);
        if (ret.empty() || id < 1) return ret;
        auto factor = Get::director->getContentScaleFactor();
        #ifdef GEODE_IS_MOBILE
        if (factor >= 4.0f) {
            auto uhdSuffix = addSuffix(Filesystem::getPathString(Icons::vanillaTexturePath(fmt::format("{}.png", ret), false)), "-uhd");
            if (isFileExist(uhdSuffix)) {
                uhdSuffix.resize(uhdSuffix.size() - 4);
                return uhdSuffix;
            }
        }
        #endif
        auto path = Filesystem::pathToString(dirs::getResourcesDir() / fmt::format(L("{}.png"), Filesystem::strWide(ret)));
        #ifndef GEODE_IS_MOBILE
        if (factor >= 4.0f) {
            auto uhdSuffix = addSuffix(path, "-uhd");
            if (isFileExist(uhdSuffix)) {
                uhdSuffix.resize(uhdSuffix.size() - 4);
                return uhdSuffix;
            }
        }
        #endif
        if (factor >= 2.0f) {
            auto hdSuffix = addSuffix(path, "-hd");
            if (isFileExist(hdSuffix)) {
                hdSuffix.resize(hdSuffix.size() - 4);
                return hdSuffix;
            }
        }
        if (isFileExist(path)) {
            path.resize(path.size() - 4);
            return path;
        }
        return ret;
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
