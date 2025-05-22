#include <cocos2d.h>
#include <Geode/GeneratedPredeclare.hpp>
#include <IconInfo.hpp>

class MoreIconsAPI {
public:
    inline static std::vector<IconInfo> icons;
    inline static std::map<IconType, std::pair<size_t, size_t>> iconIndices;
    inline static std::map<int, std::map<IconType, std::string>> requestedIcons;
    inline static std::map<std::pair<std::string, IconType>, int> loadedIcons;
    inline static bool preloadIcons = false;

    static IconInfo* getIcon(const std::string& name, IconType type);
    static IconInfo* getIcon(IconType type, bool dual);
    static std::string activeIcon(IconType type, bool dual);
    static std::string setIcon(const std::string& icon, IconType type, bool dual);
    static IconType getIconType(PlayerObject* object);
    static int getCount(IconType type);
    static bool hasIcon(const std::string& icon, IconType type);
    static std::string getFrameName(const std::string& name, const std::string& prefix, IconType type);
    static void loadIcon(const std::string& name, IconType type, int requestID);
    static void loadIconAsync(const IconInfo& info);
    static void finishLoadIcons();
    static void unloadIcon(const std::string& name, IconType type, int requestID);
    static void unloadIcons(int requestID);
    static void updateSimplePlayer(SimplePlayer* player, IconType type, bool dual);
    static void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type);
    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type);
    static void updatePlayerObject(PlayerObject* object, IconType type, bool dual);
    static void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type);
    static cocos2d::CCSpriteFrame* createSpriteFrame(cocos2d::CCDictionary* dict, cocos2d::CCTexture2D* texture, int format);
    static cocos2d::CCDictionary* createDictionary(const std::filesystem::path& path, bool async);
    static std::vector<uint8_t> getFileData(const std::filesystem::path& path);
};
