#include <cocos2d.h>
#include <fmt/format.h>
#ifdef GEODE_IS_WINDOWS
#include <fmt/xchar.h>
#endif
#include <Geode/GeneratedPredeclare.hpp>
#include <IconInfo.hpp>
#include <std23/function_ref.h>

#ifdef GEODE_IS_WINDOWS
#define WIDE_CONFIG L"config\\" GEODE_CONCAT(L, GEODE_MOD_ID)
#define L(x) L##x
#else
#define WIDE_CONFIG "config/" GEODE_MOD_ID
#define L(x) x
#endif

namespace std::filesystem {
    std::string format_as(const path& p);

    inline path operator/(path&& lhs, const path& rhs) {
        lhs /= rhs;
        return std::move(lhs);
    }
}

class MoreIcons {
public:
    static std::map<IconType, std::vector<IconInfo>> icons;
    static std::map<int, std::map<IconType, std::string>> requestedIcons;
    static std::map<std::pair<std::string, IconType>, int> loadedIcons;
    static bool traditionalPacks;
    static bool preloadIcons;

    template <typename... Args>
    static cocos2d::CCSpriteFrame* getFrame(fmt::format_string<Args...> name, Args&&... args) {
        return getFrame(fmt::format(name, std::forward<Args>(args)...).c_str());
    }
    static cocos2d::CCSpriteFrame* getFrame(const char* name);

    static void blendStreak(cocos2d::CCMotionStreak* streak, IconInfo* info);
    static cocos2d::CCTexture2D* createAndAddFrames(IconInfo* info);
    static geode::Result<std::filesystem::path> createTrash();
    static cocos2d::CCSprite* customIcon(IconInfo* info);
    static bool dualSelected();
    static bool doesExist(const std::filesystem::path& path);
    static std::filesystem::path getEditorDir(IconType type);
    static std::filesystem::path getIconDir(IconType type);
    static std::string getIconName(int id, IconType type);
    static std::pair<std::string, std::string> getIconPaths(int id, IconType type);
    static std::filesystem::path getIconStem(std::string_view name, IconType type);
    static std::basic_string_view<std::filesystem::path::value_type> getPathFilename(const std::filesystem::path& path, size_t removeCount = 0);
    static std::filesystem::path::string_type getPathString(std::filesystem::path&& path);
    static std::filesystem::path getResourcesDir(bool uhd);
    static std::string getTrailTexture(int id);
    static void iterate(
        const std::filesystem::path& path, std::filesystem::file_type type, std23::function_ref<void(const std::filesystem::path&)> func
    );
    static void loadIcons(IconType type);
    static void loadPacks();
    static void loadSettings();
    static geode::Result<std::filesystem::path> renameFile(
        const std::filesystem::path& from, const std::filesystem::path& to, bool overwrite = true, bool copy = false
    );
    static void setName(cocos2d::CCNode* node, std::string_view name);
    static std::filesystem::path strPath(std::string_view path);
    static std::string strNarrow(std::basic_string_view<std::filesystem::path::value_type> str);
    static std::filesystem::path::string_type strWide(std::string_view str);
    static void updateGarage(GJGarageLayer* layer = nullptr);
    static cocos2d::ccColor3B vanillaColor1(bool dual);
    static cocos2d::ccColor3B vanillaColor2(bool dual);
    static cocos2d::ccColor3B vanillaColorGlow(bool dual);
    static bool vanillaGlow(bool dual);
    static int vanillaIcon(IconType type, bool dual);
    static std::string vanillaTexturePath(std::string_view path, bool skipSuffix);
};
