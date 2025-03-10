#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>

struct IconInfo {
    std::string name;
    std::vector<std::string> textures;
    std::vector<std::string> frameNames;
    std::string sheetName;
    std::string packName;
    std::string packID;
    IconType type;
    int trailID;
    bool blend;
    bool tint;
};

class MoreIconsAPI {
public:
    static inline std::vector<IconInfo> ICONS;
    static inline std::vector<std::pair<std::string, IconType>> LOADED_ICONS;

    static std::vector<std::string> vectorForType(IconType type);
    static std::optional<IconInfo> infoForIcon(const std::string& name, IconType type);
    static std::string activeForType(IconType type, bool dual);
    static std::string setIcon(const std::string& icon, IconType type, bool dual);
    static int countForType(IconType type);
    static bool hasIcon(const std::string& icon, IconType type);
    static IconType getIconType(PlayerObject* object);
    static std::string getFrameName(const std::string& name, const std::string& prefix, IconType type);
    static void loadIcon(const std::string& name, IconType type);
    static void unloadIcon(const std::string& name, IconType type);
    static void updateSimplePlayer(SimplePlayer* player, IconType type, bool dual);
    static void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type);
    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type);
    static void updatePlayerObject(PlayerObject* object, IconType type, bool dual);
    static void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type);
};
