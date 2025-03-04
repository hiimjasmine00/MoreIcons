#include "api/MoreIconsAPI.hpp"

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

struct ImageData {
    cocos2d::CCImage* image;
    cocos2d::CCDictionary* dict;
    std::string texturePath;
    std::string name;
    std::string frameName;
    TexturePack pack;
    int index;
    int trailID;
    bool blend;
    bool tint;
};

enum class LogType {
    Info,
    Warn,
    Error
};

struct LogData {
    std::string message;
    LogType type;
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
    static inline std::vector<ImageData> IMAGES;
    static inline std::mutex IMAGE_MUTEX;
    static inline std::vector<LogData> LOGS;
    static inline std::mutex LOG_MUTEX;
    static inline LogType HIGHEST_SEVERITY = LogType::Info;
    static inline bool GLOBED_ICONS_LOADED = false;

    static void naturalSort(std::vector<std::string>& vec);
    static bool naturalSorter(const std::string& aStr, const std::string& bStr);
    static std::vector<IconPack> getTexturePacks();
    static void unzipVanillaAssets();
    static std::string vanillaTexturePath(const std::string& path, bool skipSuffix);
    static void loadIcons(const std::vector<IconPack>& packs, std::string_view suffix, IconType type);
    static void loadIcon(const std::filesystem::path& path, const IconPack& pack, IconType type);
    static void loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack, IconType type);
    static void loadTrails(const std::vector<IconPack>& packs);
    static void loadTrail(const std::filesystem::path& path, const IconPack& pack);
    static void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack);
    static void saveTrails();
    static bool dualSelected();

    static void changeSimplePlayer(SimplePlayer* player, IconType type) {
        MoreIconsAPI::updateSimplePlayer(player, MoreIconsAPI::activeForType(type, dualSelected()), type);
    }

    static void changeSimplePlayer(SimplePlayer* player, IconType type, bool dual) {
        MoreIconsAPI::updateSimplePlayer(player, MoreIconsAPI::activeForType(type, dual), type);
    }

    static std::unordered_map<std::string, TexturePack>& infoForType(IconType type) {
        switch (type) {
            case IconType::Cube: return ICON_INFO;
            case IconType::Ship: return SHIP_INFO;
            case IconType::Ball: return BALL_INFO;
            case IconType::Ufo: return UFO_INFO;
            case IconType::Wave: return WAVE_INFO;
            case IconType::Robot: return ROBOT_INFO;
            case IconType::Spider: return SPIDER_INFO;
            case IconType::Swing: return SWING_INFO;
            case IconType::Jetpack: return JETPACK_INFO;
            default: {
                static std::unordered_map<std::string, TexturePack> empty;
                return empty;
            }
        }
    }

    static void showInfoPopup(bool folderButton = false);
};
