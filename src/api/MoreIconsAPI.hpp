#include <cocos2d.h>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/Result.hpp>
#include <IconInfo.hpp>

struct ImageResult {
    std::vector<uint8_t> data;
    cocos2d::CCTexture2D* texture;
    cocos2d::CCDictionary* frames;
    uint32_t width;
    uint32_t height;
};

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
    static cocos2d::CCTexture2D* loadIcon(const std::string& name, IconType type, int requestID);
    static void loadIconAsync(IconInfo* info);
    static int finishLoadIcons();
    static void unloadIcon(const std::string& name, IconType type, int requestID);
    static void unloadIcons(int requestID);
    static void updateSimplePlayer(SimplePlayer* player, IconType type, bool dual);
    static void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type);
    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type);
    static void updatePlayerObject(PlayerObject* object, IconType type, bool dual);
    static void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type);
    static geode::Result<std::vector<uint8_t>> getFileData(const std::string& path);
    static geode::Result<ImageResult> packFrames(const std::vector<std::string>& textures, const std::vector<std::string>& frameNames);
    static geode::Result<ImageResult> createFrames(const std::string& png, const std::string& plist, const std::string& name, IconType type);
    static geode::Result<std::pair<cocos2d::CCTexture2D*, cocos2d::CCDictionary*>> createFrames(
        const std::string& path, cocos2d::CCTexture2D* texture, const std::string& name, IconType type, bool fixNames = true
    );
    static std::vector<std::string> addFrames(const std::string& name, const ImageResult& image);
};
