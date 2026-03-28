#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/loader/Types.hpp>

#ifdef GEODE_IS_WINDOWS
#define CONFIG_PATH std::wstring_view(L"config\\" GEODE_CONCAT(L, GEODE_MOD_ID), sizeof(GEODE_MOD_ID) + 6)
#else
#define CONFIG_PATH std::string_view("config/" GEODE_MOD_ID, sizeof(GEODE_MOD_ID) + 6)
#endif

class IconInfo;
class IconWrapper;

namespace Icons {
    extern std::map<int, std::map<IconType, IconInfo*>> requestedIcons;
    extern std::unordered_map<IconInfo*, int> loadedIcons;
    extern std::unordered_map<IconWrapper*, IconInfo*> iconWrappers;
    extern std::vector<geode::Hook*> hooks;
    extern geode::Hook* deathEffectHook;
    extern bool traditionalPacks;
    extern bool preloadIcons;
    extern bool loadingFinished;

    cocos2d::CCSpriteFrame* getFrame(const std::string& name);
    template <typename... Args>
    cocos2d::CCSpriteFrame* getFrame(fmt::format_string<Args...> name, Args&&... args) {
        return getFrame(fmt::format(name, std::forward<Args>(args)...));
    }

    cocos2d::CCTexture2D* createAndAddFrames(IconInfo* info);
    void finishLoading();
    #ifdef GEODE_IS_MOBILE
    std::filesystem::path getUhdResourcesDir();
    #endif
    void loadIcons(IconType type);
    void loadPacks();
    void loadSettings();
    void setIcon(cocos2d::CCNode* node, IconInfo* info);
    void uncacheIcon(IconInfo* info);
    std::filesystem::path vanillaTexturePath(std::basic_string_view<std::filesystem::path::value_type> path, bool skipSuffix);
}

class IconWrapper : public cocos2d::CCObject {
protected:
    IconInfo* m_info = nullptr;
public:
    static IconWrapper* create(IconInfo* info);

    void setInfo(IconInfo* info);

    ~IconWrapper();
};
