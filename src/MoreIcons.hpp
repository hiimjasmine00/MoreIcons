#include <array>
#include <ccTypes.h>
#include <filesystem>
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/Result.hpp>
#include <map>

struct IconPack {
    std::string name;
    std::string id;
    std::filesystem::path path;
    bool vanilla;
    bool zipped;
};

struct LogData {
    std::string name;
    std::string message;
    IconType type;
    int severity;
};

struct ColorInfo {
    cocos2d::ccColor3B color1;
    cocos2d::ccColor3B color2;
    cocos2d::ccColor3B colorGlow;
    bool glow;
};

class MoreIcons {
public:
    static constexpr std::array severityFrames = {
        "cc_2x2_white_image", "GJ_infoIcon_001.png", "geode.loader/info-warning.png", "geode.loader/info-alert.png"
    };
    static constexpr std::array folders = {
        "icon", "ship", "ball", "ufo", "wave", "robot", "spider",
        "swing", "jetpack", "death", "trail", "", "fire"
    };
    static std::vector<IconPack> packs;
    static std::vector<LogData> logs;
    static std::map<IconType, int> severities;
    static int severity;
    static bool traditionalPacks;

    static geode::Result<std::filesystem::path> createTrash();
    static bool doesExist(const std::filesystem::path& path);
    static void loadIcons(IconType type);
    static void loadPacks();
    static void loadSettings();
    static geode::Result<> renameFile(const std::filesystem::path& from, const std::filesystem::path& to, bool overwrite = true, bool copy = false);
    static void saveTrails();
    static void updateGarage(GJGarageLayer* layer = nullptr);
    static ColorInfo vanillaColors(bool dual);
    static int vanillaIcon(IconType type, bool dual);
    static std::string vanillaTexturePath(const std::string& path, bool skipSuffix);
};
