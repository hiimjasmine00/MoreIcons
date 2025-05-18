#include <filesystem>
#include <Geode/Enums.hpp>
#include <map>

struct IconPack {
    std::string name;
    std::string id;
    std::filesystem::path path;
    bool vanilla;
};

struct LogData {
    std::string name;
    std::string message;
    IconType type;
    int severity;
};

class MoreIcons {
public:
    inline static std::vector<LogData> logs;
    inline static constexpr std::array severityFrames = {
        "cc_2x2_white_image", "GJ_infoIcon_001.png", "geode.loader/info-warning.png", "geode.loader/info-alert.png"
    };
    inline static constexpr std::array prefixes = {
        "player_", "ship_", "player_ball_", "bird_", "dart_", "robot_", "spider_", "swing_", "jetpack_"
    };
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
    inline static IconType currentType = IconType::Cube;
    inline static int severity = 0;
    inline static bool showReload = false;
    inline static bool debugLogs = true;
    inline static bool traditionalPacks = true;

    static std::vector<IconPack> getTexturePacks();
    static std::string vanillaTexturePath(const std::string& path, bool skipSuffix);
    static void loadIcons(const std::vector<IconPack>& packs, std::string_view suffix, IconType type);
    static void loadTrails(const std::vector<IconPack>& packs);
    static void saveTrails();
};
