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
    inline static constexpr std::array severityFrames = {
        "cc_2x2_white_image", "GJ_infoIcon_001.png", "geode.loader/info-warning.png", "geode.loader/info-alert.png"
    };
    inline static constexpr std::array folders = {
        "icon", "ship", "ball", "ufo", "wave", "robot", "spider",
        "swing", "jetpack", "death", "trail", "", "fire"
    };
    inline static constexpr std::array uppercase = {
        "Icon", "Ship", "Ball", "UFO", "Wave", "Robot", "Spider",
        "Swing", "Jetpack", "Death Effect", "Trail", "", "Ship Fire"
    };
    inline static std::vector<IconPack> packs;
    inline static std::vector<LogData> logs;
    inline static std::map<IconType, int> severities = {
        { IconType::Cube, 0 },
        { IconType::Ship, 0 },
        { IconType::Ball, 0 },
        { IconType::Ufo, 0 },
        { IconType::Wave, 0 },
        { IconType::Robot, 0 },
        { IconType::Spider, 0 },
        { IconType::Swing, 0 },
        { IconType::Jetpack, 0 },
        { IconType::Special, 0 }
    };
    inline static int severity = 0;
    inline static bool traditionalPacks = true;

    static ColorInfo activeColors(bool dual);
    static int activeIcon(IconType type, bool dual);
    static geode::Result<std::filesystem::path> createTrash();
    static bool doesExist(const std::filesystem::path& path);
    static void loadIcons(IconType type);
    static void loadPacks();
    static void loadSettings();
    static void saveTrails();
    static void updateGarage(GJGarageLayer* layer = nullptr);
    static std::string vanillaTexturePath(const std::string& path, bool skipSuffix);
};
