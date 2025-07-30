#include <cocos2d.h>
#include <Geode/GeneratedPredeclare.hpp>
#include <IconInfo.hpp>

template <class T>
struct Autorelease {
    T* data;

    Autorelease() : data(nullptr) {}
    Autorelease(T* data) : data(data) {}
    Autorelease(const Autorelease& other) : data(other.data) {}
    Autorelease(Autorelease&& other) : data(other.data) {
        other.data = nullptr;
    }

    ~Autorelease() {
        CC_SAFE_RELEASE(data);
    }

    Autorelease& operator=(T* obj) {
        if (data != obj) {
            CC_SAFE_RELEASE(data);
            data = obj;
            CC_SAFE_RETAIN(data);
        }
        return *this;
    }
    Autorelease& operator=(const Autorelease& other) {
        if (this != &other) {
            CC_SAFE_RELEASE(data);
            data = other.data;
            CC_SAFE_RETAIN(data);
        }
        return *this;
    }
    Autorelease& operator=(Autorelease&& other) {
        if (this != &other) {
            data = other.data;
            other.data = nullptr;
        }
        return *this;
    }

    operator T*() const {
        return data;
    }
    T* operator->() const {
        return data;
    }

};

struct ImageResult {
    std::string name;
    std::vector<uint8_t> data;
    Autorelease<cocos2d::CCTexture2D> texture;
    Autorelease<cocos2d::CCDictionary> frames;
    uint32_t width;
    uint32_t height;
};

class MoreIconsAPI {
public:
    inline static constexpr std::array prefixes = {
        "player_", "ship_", "player_ball_", "bird_", "dart_", "robot_", "spider_",
        "swing_", "jetpack_", "PlayerExplosion_", "streak_", "", "shipfire"
    };
    inline static constexpr std::array lowercase = {
        "icon", "ship", "ball", "UFO", "wave", "robot", "spider",
        "swing", "jetpack", "death effect", "trail", "", "ship fire"
    };
    inline static constexpr std::array uppercase = {
        "Icon", "Ship", "Ball", "UFO", "Wave", "Robot", "Spider",
        "Swing", "Jetpack", "Death Effect", "Trail", "", "Ship Fire"
    };
    inline static std::map<IconType, std::vector<IconInfo>> icons = {
        { IconType::Cube, {} },
        { IconType::Ship, {} },
        { IconType::Ball, {} },
        { IconType::Ufo, {} },
        { IconType::Wave, {} },
        { IconType::Robot, {} },
        { IconType::Spider, {} },
        { IconType::Swing, {} },
        { IconType::Jetpack, {} },
        { IconType::Special, {} }
    };
    inline static std::map<int, std::map<IconType, std::string>> requestedIcons;
    inline static std::map<std::pair<std::string, IconType>, int> loadedIcons;
    inline static bool preloadIcons = false;

    static IconInfo* getIcon(const std::string& name, IconType type);
    static IconInfo* getIcon(IconType type, bool dual);
    static std::string activeIcon(IconType type, bool dual);
    static std::string setIcon(const std::string& icon, IconType type, bool dual);
    static IconType getIconType(PlayerObject* object);
    static std::string getIconName(cocos2d::CCNode* node);
    static bool hasIcon(const std::string& icon, IconType type);
    static bool hasIcon(IconType type, bool dual);
    static int convertType(IconType type) {
        return (int)type - (type >= IconType::DeathEffect) * 89;
    }
    template <class T>
    static T* get();
    static void reset();
    static cocos2d::CCSpriteFrame* getFrame(std::string_view name);
    static cocos2d::CCSprite* customTrail(const std::string& png);
    static cocos2d::CCTexture2D* loadIcon(const std::string& name, IconType type, int requestID);
    static void loadIcons(IconType type, bool logs);
    static void unloadIcon(const std::string& name, IconType type, int requestID);
    static void unloadIcons(int requestID);
    static IconInfo* addIcon(
        const std::string& name, const std::string& shortName, IconType type, const std::string& png, const std::string& plist,
        const std::string& packID, const std::string& packName, int trailID, const TrailInfo& trailInfo, bool vanilla, bool zipped
    );
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
    static geode::Result<Autorelease<cocos2d::CCDictionary>> createFrames(
        const std::string& path, cocos2d::CCTexture2D* texture, const std::string& name, IconType type, bool fixNames = true
    );
    static cocos2d::CCTexture2D* addFrames(const ImageResult& image, std::vector<std::string>& frameNames);
};
