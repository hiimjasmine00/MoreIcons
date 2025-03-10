#include "api/MoreIconsAPI.hpp"
#include <Geode/loader/Types.hpp>

struct IconPack {
    std::string name;
    std::string id;
    std::filesystem::path path;
    bool vanilla;
};

struct LogData {
    std::string message;
    geode::Severity severity;
};

class MoreIcons {
public:
    static inline std::vector<LogData> LOGS;
    static inline geode::Severity HIGHEST_SEVERITY = geode::Severity::Debug;
    static inline bool DEBUG_LOGS = true;
    static inline bool TRADITIONAL_PACKS = true;

    static std::vector<IconPack> getTexturePacks();
    static std::string vanillaTexturePath(const std::string& path, bool skipSuffix);
    static void loadIcons(const std::vector<IconPack>& packs, std::string_view suffix, IconType type);
    static void loadTrails(const std::vector<IconPack>& packs);
    static void saveTrails();
    static void showInfoPopup(bool folderButton = false);
};
