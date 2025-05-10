#define MoreIcons _MoreIcons
#include <MoreIcons.hpp>
#undef MoreIcons
#ifndef MORE_ICONS_NO_ALIAS
using MoreIcons = _MoreIcons;
#endif

class MoreIconsAPI {
public:
    inline static std::vector<IconInfo> icons;
    inline static std::map<IconType, std::pair<size_t, size_t>> iconIndices;
    inline static std::map<int, std::map<IconType, std::string>> requestedIcons;
    inline static std::map<std::pair<std::string, IconType>, int> loadedIcons;

    static IconInfo* getIcon(const std::string& name, IconType type);
    static int getCount(IconType type);
    static bool hasIcon(const std::string& icon, IconType type);
    static std::string getFrameName(const std::string& name, const std::string& prefix, IconType type);
    static void loadIcon(const std::string& name, IconType type, int requestID);
    static void unloadIcon(const std::string& name, IconType type, int requestID);
    static void unloadIcons(int requestID);
    static void updateSimplePlayer(SimplePlayer* player, IconType type, bool dual);
    static void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type);
    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type);
    static void updatePlayerObject(PlayerObject* object, IconType type, bool dual);
    static void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type);
    static cocos2d::CCSpriteFrame* createSpriteFrame(cocos2d::CCDictionary* dict, cocos2d::CCTexture2D* texture, int format);
};
