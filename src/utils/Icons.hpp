#include <cocos2d.h>
#include <fmt/format.h>
#include <IconInfo.hpp>

#ifdef GEODE_IS_WINDOWS
#define CONFIG_PATH std::wstring_view(L"config\\" GEODE_CONCAT(L, GEODE_MOD_ID), sizeof(GEODE_MOD_ID) + 6)
#else
#define CONFIG_PATH std::string_view("config/" GEODE_MOD_ID, sizeof(GEODE_MOD_ID) + 6)
#endif

struct IconPack {
    std::string name;
    std::string id;
    std::filesystem::path path;
    bool vanilla;
    bool zipped;
};

class Icons {
public:
    static std::map<IconType, std::vector<IconInfo>> icons;
    static std::map<int, std::map<IconType, std::string>> requestedIcons;
    static std::map<std::pair<std::string, IconType>, int> loadedIcons;
    static std::vector<IconPack> packs;
    static bool traditionalPacks;
    static bool preloadIcons;

    template <typename... Args>
    static cocos2d::CCSpriteFrame* getFrame(fmt::format_string<Args...> name, Args&&... args) {
        return getFrame(fmt::format(name, std::forward<Args>(args)...).c_str());
    }
    static cocos2d::CCSpriteFrame* getFrame(const char* name);

    static cocos2d::CCTexture2D* createAndAddFrames(IconInfo* info);
    #ifdef GEODE_IS_MOBILE
    static std::filesystem::path getUhdResourcesDir();
    #endif
    static void loadIcons(IconType type);
    static void loadPacks();
    static void loadSettings();
    static void setName(cocos2d::CCNode* node, std::string_view name);
    static std::string vanillaTexturePath(std::string_view path, bool skipSuffix);
};
