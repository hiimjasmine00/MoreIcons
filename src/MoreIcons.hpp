#include "api/MoreIconsAPI.hpp"
#include <Geode/loader/Types.hpp>

struct TexturePack {
    std::string name;
    std::string id;
};

struct IconPack {
    std::string name;
    std::string id;
    std::filesystem::path path;
    bool vanilla;
};

struct TrailInfo {
    std::string texture;
    TexturePack pack;
    int trailID;
    bool blend;
    bool tint;
};

struct LogData {
    std::string message;
    geode::Severity severity;
};

class MoreIcons {
public:
    static inline std::unordered_map<std::string, TexturePack> ICON_INFO;
    static inline std::unordered_map<std::string, TexturePack> SHIP_INFO;
    static inline std::unordered_map<std::string, TexturePack> BALL_INFO;
    static inline std::unordered_map<std::string, TexturePack> UFO_INFO;
    static inline std::unordered_map<std::string, TexturePack> WAVE_INFO;
    static inline std::unordered_map<std::string, TexturePack> ROBOT_INFO;
    static inline std::unordered_map<std::string, TexturePack> SPIDER_INFO;
    static inline std::unordered_map<std::string, TexturePack> SWING_INFO;
    static inline std::unordered_map<std::string, TexturePack> JETPACK_INFO;
    static inline std::unordered_map<std::string, TrailInfo> TRAIL_INFO;
    static inline std::vector<LogData> LOGS;
    static inline geode::Severity HIGHEST_SEVERITY = geode::Severity::Info;
    static inline bool DEBUG_LOGS = true;
    static inline bool TRADITIONAL_PACKS = true;

    static std::vector<IconPack> getTexturePacks();
    GEODE_ANDROID(static void unzipVanillaAssets();)
    static std::string vanillaTexturePath(const std::string& path, bool skipSuffix);
    static void loadIcons(const std::vector<IconPack>& packs, std::string_view suffix, IconType type);
    static void loadTrails(const std::vector<IconPack>& packs);
    static void saveTrails();
    static std::unordered_map<std::string, TexturePack>& infoForType(IconType type);
    static void showInfoPopup(bool folderButton = false);
};
