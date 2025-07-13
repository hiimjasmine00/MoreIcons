#include <Geode/utils/cocos.hpp>
#include <IconInfo.hpp>
#include <span>

struct ImageResult {
    std::string name;
    std::vector<uint8_t> data;
    geode::Ref<cocos2d::CCTexture2D> texture;
    geode::Ref<cocos2d::CCDictionary> frames;
    uint32_t width;
    uint32_t height;
};

class MoreIconsAPI {
public:
    inline static std::vector<IconInfo> icons;
    inline static std::map<IconType, std::span<IconInfo>> iconSpans;
    inline static std::map<int, std::map<IconType, std::string>> requestedIcons;
    inline static std::map<std::pair<std::string, IconType>, int> loadedIcons;
    inline static bool preloadIcons = false;

    static IconInfo* getIcon(const std::string& name, IconType type);
    static IconInfo* getIcon(IconType type, bool dual);
    static std::string activeIcon(IconType type, bool dual);
    static std::string setIcon(const std::string& icon, IconType type, bool dual);
    static IconType getIconType(PlayerObject* object);
    static std::string getIconName(cocos2d::CCNode* node);
    static int getCount(IconType type);
    static bool hasIcon(const std::string& icon, IconType type);
    static bool hasIcon(IconType type, bool dual);
    static std::string iconName(int id, IconType type);
    static std::string iconName(int id, UnlockType type);
    template <class T>
    static geode::Ref<T> createRef() {
        geode::Ref ret = new T();
        ret->release();
        return ret;
    }
    template <typename... T>
    static cocos2d::CCSpriteFrame* getFrame(fmt::format_string<T...> format, T&&... args) {
        return getFrameInternal(format, fmt::make_format_args(args...));
    }
    static cocos2d::CCSpriteFrame* getFrameInternal(fmt::string_view format, fmt::format_args args);
    static cocos2d::CCSprite* customTrail(const std::string& png);
    static cocos2d::CCTexture2D* loadIcon(const std::string& name, IconType type, int requestID);
    static void loadIcons(IconType type, bool logs);
    static void unloadIcon(const std::string& name, IconType type, int requestID);
    static void unloadIcons(int requestID);
    static void addIcon(IconInfo&& info, bool postLoad = false);
    static void moveIcon(IconInfo* info, const std::filesystem::path& path);
    static void removeIcon(IconInfo* info);
    static void renameIcon(IconInfo* info, const std::string& name);
    static void updateIcon(IconInfo* info);
    static void updateSimplePlayer(SimplePlayer* player, IconType type, bool dual, bool load = true);
    static void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type, bool load = true);
    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type, bool load = true);
    static void updatePlayerObject(PlayerObject* object, IconType type, bool dual);
    static void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type);
    static geode::Result<ImageResult> createFrames(const std::string& png, const std::string& plist, const std::string& name, IconType type);
    static geode::Result<geode::Ref<cocos2d::CCDictionary>> createFrames(
        const std::string& path, cocos2d::CCTexture2D* texture, const std::string& name, IconType type, bool fixNames = true
    );
    static cocos2d::CCTexture2D* addFrames(ImageResult image, std::vector<std::string>& frameNames);
};
