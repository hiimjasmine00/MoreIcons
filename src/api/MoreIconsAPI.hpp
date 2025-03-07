#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>

class MoreIconsAPI {
public:
    static inline std::vector<std::string> ICONS;
    static inline std::vector<std::string> SHIPS;
    static inline std::vector<std::string> BALLS;
    static inline std::vector<std::string> UFOS;
    static inline std::vector<std::string> WAVES;
    static inline std::vector<std::string> ROBOTS;
    static inline std::vector<std::string> SPIDERS;
    static inline std::vector<std::string> SWINGS;
    static inline std::vector<std::string> JETPACKS;
    static inline std::vector<std::string> TRAILS;

    static std::vector<std::string>& vectorForType(IconType type);
    static std::string activeForType(IconType type, bool dual);
    static std::string setIcon(const std::string& icon, IconType type, bool dual);
    static bool hasIcon(const std::string& icon, IconType type);
    static IconType getIconType(PlayerObject* object);
    static void updateSimplePlayer(SimplePlayer* player, IconType type, bool dual);
    static void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type);
    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type);
    static void updatePlayerObject(PlayerObject* object, IconType type, bool dual);
    static void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type);
};
