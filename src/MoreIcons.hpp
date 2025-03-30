#define MORE_ICONS_NO_ALIAS

#include <filesystem>
#include <Geode/Enums.hpp>
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
    inline static std::vector<LogData> logs;
    inline static geode::Severity severity = geode::Severity::Debug;
    inline static bool debugLogs = true;
    inline static bool traditionalPacks = true;

    static std::vector<IconPack> getTexturePacks();
    static std::string vanillaTexturePath(const std::string& path, bool skipSuffix);
    static void loadIcons(const std::vector<IconPack>& packs, std::string_view suffix, IconType type);
    static void loadTrails(const std::vector<IconPack>& packs);
    static void saveTrails();
    static void showInfoPopup(bool folderButton = false);
};
