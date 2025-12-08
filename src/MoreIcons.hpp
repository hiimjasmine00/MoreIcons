#include <cocos2d.h>
#include <fmt/format.h>
#include <Geode/GeneratedPredeclare.hpp>
#include <IconInfo.hpp>
#include <std23/function_ref.h>

struct LogData {
    std::string name;
    std::string message;
    IconType type;
    int severity;
};

#ifdef GEODE_IS_WINDOWS
#define MI_PATH_ID GEODE_CONCAT(L, GEODE_MOD_ID)
#define MI_PATH(x) L##x
#else
#define MI_PATH_ID GEODE_MOD_ID
#define MI_PATH(x) x
#endif

namespace std::filesystem {
    std::string format_as(const path& p);
}

class MoreIcons {
public:
    static constexpr std::array prefixes = {
        "player_", "ship_", "player_ball_", "bird_", "dart_", "robot_", "spider_",
        "swing_", "jetpack_", "PlayerExplosion_", "streak_", "", "shipfire"
    };
    static constexpr std::array lowercase = {
        "icon", "ship", "ball", "UFO", "wave", "robot", "spider",
        "swing", "jetpack", "death effect", "trail", "", "ship fire"
    };
    static constexpr std::array uppercase = {
        "Icon", "Ship", "Ball", "UFO", "Wave", "Robot", "Spider",
        "Swing", "Jetpack", "Death Effect", "Trail", "", "Ship Fire"
    };
    static constexpr std::array severityFrames = {
        "cc_2x2_white_image", "GJ_infoIcon_001.png", "geode.loader/info-warning.png", "geode.loader/info-alert.png"
    };
    #ifdef GEODE_IS_WINDOWS
    static constexpr std::array folders = {
        L"icon", L"ship", L"ball", L"ufo", L"wave", L"robot", L"spider",
        L"swing", L"jetpack", L"death", L"trail", L"", L"fire"
    };
    #else
    static constexpr std::array folders = {
        "icon", "ship", "ball", "ufo", "wave", "robot", "spider",
        "swing", "jetpack", "death", "trail", "", "fire"
    };
    #endif

    static std::map<IconType, std::vector<IconInfo>> icons;
    static std::map<int, std::map<IconType, std::string>> requestedIcons;
    static std::map<std::pair<std::string, IconType>, int> loadedIcons;
    static std::vector<LogData> logs;
    static std::map<IconType, int> severities;
    static int severity;
    static bool traditionalPacks;
    static bool preloadIcons;

    static int convertType(IconType type) {
        return (int)type - (type >= IconType::DeathEffect ? 89 : 0);
    }

    static IconType convertType(int type) {
        return (IconType)(type + (type > 8 ? 89 : 0));
    }

    template <typename... Args>
    static void notifyFailure(fmt::format_string<Args...> message, Args&&... args) {
        notifyFailure(fmt::format(message, std::forward<Args>(args)...));
    }
    static void notifyFailure(const std::string& message);

    template <typename... Args>
    static void notifyInfo(fmt::format_string<Args...> message, Args&&... args) {
        notifyInfo(fmt::format(message, std::forward<Args>(args)...));
    }
    static void notifyInfo(const std::string& message);

    template <typename... Args>
    static void notifySuccess(fmt::format_string<Args...> message, Args&&... args) {
        notifySuccess(fmt::format(message, std::forward<Args>(args)...));
    }
    static void notifySuccess(const std::string& message);

    template <typename... Args>
    static cocos2d::CCSpriteFrame* getFrame(fmt::format_string<Args...> name, Args&&... args) {
        return getFrame(fmt::format(name, std::forward<Args>(args)...).c_str());
    }
    static cocos2d::CCSpriteFrame* getFrame(const char* name);

    static cocos2d::CCTexture2D* createAndAddFrames(IconInfo* info);
    static geode::Result<std::filesystem::path> createTrash();
    static cocos2d::CCSprite* customTrail(const char* png);
    static bool dualSelected();
    static bool doesExist(const std::filesystem::path& path);
    static std::filesystem::path getEditorDir(IconType type);
    static std::filesystem::path getIconDir(IconType type);
    static std::filesystem::path getIconStem(const std::string& name, IconType type);
    static std::filesystem::path::string_type getPathString(std::filesystem::path path);
    static TrailInfo getTrailInfo(int trailID);
    static void iterate(
        const std::filesystem::path& path, std::filesystem::file_type type, std23::function_ref<void(const std::filesystem::path&)> func
    );
    static void loadIcons(IconType type);
    static void loadPacks();
    static void loadSettings();
    static geode::Result<std::filesystem::path> renameFile(
        const std::filesystem::path& from, const std::filesystem::path& to, bool overwrite = true, bool copy = false
    );
    static std::filesystem::path strPath(const std::string& path);
    static void updateGarage(GJGarageLayer* layer = nullptr);
    static cocos2d::ccColor3B vanillaColor1(bool dual);
    static cocos2d::ccColor3B vanillaColor2(bool dual);
    static cocos2d::ccColor3B vanillaColorGlow(bool dual);
    static bool vanillaGlow(bool dual);
    static int vanillaIcon(IconType type, bool dual);
    static std::string vanillaTexturePath(const std::string& path, bool skipSuffix);
};
