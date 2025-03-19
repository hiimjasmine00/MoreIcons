#include <MoreIcons.hpp>

class MoreIconsAPI {
public:
    static inline std::vector<IconInfo> ICONS;
    static inline std::map<IconType, std::pair<int, int>> ICON_INDICES;
    static inline std::vector<std::pair<std::string, IconType>> LOADED_ICONS;

    static IconInfo* getIcon(const std::string& name, IconType type);
    static int getCount(IconType type);
    static bool hasIcon(const std::string& icon, IconType type);
    static std::string getFrameName(const std::string& name, const std::string& prefix, IconType type);
    static void loadIcon(const std::string& name, IconType type);
    static void unloadIcon(const std::string& name, IconType type);
    static void updateSimplePlayer(SimplePlayer* player, IconType type, bool dual);
    static void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type);
    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type);
    static void updatePlayerObject(PlayerObject* object, IconType type, bool dual);
    static void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type);
};
