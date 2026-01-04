#include <cocos2d.h>
#include <fmt/format.h>
#include <IconInfo.hpp>

#ifdef GEODE_IS_WINDOWS
#define CONFIG_PATH std::wstring_view(L"config\\" GEODE_CONCAT(L, GEODE_MOD_ID), sizeof(GEODE_MOD_ID) + 6)
#else
#define CONFIG_PATH std::string_view("config/" GEODE_MOD_ID, sizeof(GEODE_MOD_ID) + 6)
#endif

namespace Icons {
    extern std::map<IconType, std::vector<IconInfo>> icons;
    extern std::map<int, std::map<IconType, std::string>> requestedIcons;
    extern std::map<std::pair<std::string, IconType>, int> loadedIcons;
    extern bool traditionalPacks;
    extern bool preloadIcons;

    cocos2d::CCSpriteFrame* getFrame(const char* name);
    template <typename... Args>
    cocos2d::CCSpriteFrame* getFrame(fmt::format_string<Args...> name, Args&&... args) {
        return getFrame(fmt::format(name, std::forward<Args>(args)...).c_str());
    }

    cocos2d::CCTexture2D* createAndAddFrames(IconInfo* info);
    void finishLoading();
    #ifdef GEODE_IS_MOBILE
    std::filesystem::path getUhdResourcesDir();
    #endif
    void loadIcons(IconType type);
    void loadPacks();
    void loadSettings();
    void setName(cocos2d::CCNode* node, std::string_view name);
    std::string vanillaTexturePath(std::string_view path, bool skipSuffix);
}
