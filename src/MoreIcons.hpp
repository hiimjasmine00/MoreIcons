#include "api/MoreIconsAPI.hpp"
#include <BS_thread_pool.hpp>

struct TexturePack {
    std::string name;
    std::string id;
};

struct TrailInfo {
    std::string texture;
    TexturePack pack;
    bool blend;
    bool tint;
};

struct ImageData {
    cocos2d::CCImage* image;
    cocos2d::CCDictionary* dict;
    std::string texturePath;
    std::string name;
    std::string frameName;
    TexturePack pack;
    int index;
    bool blend;
    bool tint;
};

enum class LogType {
    Info,
    Warn,
    Error
};

struct LogData {
    std::string message;
    LogType type;
};

// https://github.com/GlobedGD/globed2/blob/v1.6.2/src/util/cocos.cpp#L44
namespace {
    template <typename TC>
    using priv_method_t = void(TC::*)(cocos2d::CCDictionary*, cocos2d::CCTexture2D*);

    template <typename TC, priv_method_t<TC> func>
    struct priv_caller {
        friend void _addSpriteFramesWithDictionary(cocos2d::CCDictionary* dict, cocos2d::CCTexture2D* texture) {
            (cocos2d::CCSpriteFrameCache::get()->*func)(dict, texture);
        }
    };

    template struct priv_caller<cocos2d::CCSpriteFrameCache, &cocos2d::CCSpriteFrameCache::addSpriteFramesWithDictionary>;

    void _addSpriteFramesWithDictionary(cocos2d::CCDictionary*, cocos2d::CCTexture2D*);
}

class MoreIcons {
public:
    static inline std::unordered_map<std::string, TexturePack> ICON_INFO;
    static inline std::unordered_map<std::string, TexturePack> SHIP_INFO;
    static inline std::unordered_map<std::string, TexturePack> BALL_INFO;
    static inline std::unordered_map<std::string, TexturePack> UFO_INFO;
    static inline std::unordered_map<std::string, TexturePack> WAVE_INFO;
    static inline std::unordered_map<std::string, TexturePack> ROBOT_INFO;
    static inline std::unordered_map<std::string, TexturePack> SPIDER_INFO;
    static inline std::unordered_map<std::string, TexturePack> SWING_INFO;
    static inline std::unordered_map<std::string, TexturePack> JETPACK_INFO;
    static inline std::unordered_map<std::string, TrailInfo> TRAIL_INFO;
    static inline std::vector<ImageData> IMAGES;
    static inline std::mutex IMAGE_MUTEX;
    static inline std::vector<LogData> LOGS;
    static inline std::mutex LOG_MUTEX;
    static inline LogType HIGHEST_SEVERITY = LogType::Info;

    static BS::thread_pool& sharedPool() {
        static BS::thread_pool _sharedPool(std::thread::hardware_concurrency());
        return _sharedPool;
    }

    static void removeSaved() {
        geode::Mod::get()->setSavedValue<std::vector<std::string>>("icons", {});
        geode::Mod::get()->setSavedValue<std::vector<std::string>>("ships", {});
        geode::Mod::get()->setSavedValue<std::vector<std::string>>("balls", {});
        geode::Mod::get()->setSavedValue<std::vector<std::string>>("ufos", {});
        geode::Mod::get()->setSavedValue<std::vector<std::string>>("waves", {});
        geode::Mod::get()->setSavedValue<std::vector<std::string>>("robots", {});
        geode::Mod::get()->setSavedValue<std::vector<std::string>>("spiders", {});
        geode::Mod::get()->setSavedValue<std::vector<std::string>>("swings", {});
        geode::Mod::get()->setSavedValue<std::vector<std::string>>("jetpacks", {});
        geode::Mod::get()->setSavedValue<std::vector<std::string>>("trails", {});
    }

    static std::vector<std::filesystem::directory_entry> naturalSort(const std::filesystem::path& path);

    static void naturalSort(std::vector<std::string>& vec);

    static bool naturalSorter(const std::string& aStr, const std::string& bStr);

    static std::vector<std::filesystem::path> getTexturePacks();

    static std::string replaceEnd(const std::string& str, std::string_view end, std::string_view replace, bool check = false) {
        return !check || str.ends_with(end) ? fmt::format("{}{}", str.substr(0, str.size() - end.size()), replace) : str;
    }

    static void loadIcons(
        const std::vector<std::filesystem::path>& packs, std::string_view suffix, IconType type
    );

    static void loadIcon(const std::filesystem::path& path, const TexturePack& pack, IconType type);

    static void loadTrails(const std::vector<std::filesystem::path>& packs);

    static void loadTrail(const std::filesystem::path& path, const TexturePack& pack);

    static void saveTrails() {
        for (auto& [trail, info] : TRAIL_INFO) {
            std::fstream file(std::filesystem::path(info.texture).replace_extension(".json"), std::ios::out);
            file << matjson::Value(matjson::makeObject({
                { "blend", info.blend },
                { "tint", info.tint },
            })).dump();
            file.close();
        }
    }

    static void changeSimplePlayer(SimplePlayer* player, IconType type) {
        auto sdi = geode::Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        MoreIconsAPI::updateSimplePlayer(player, MoreIconsAPI::activeForType(type, sdi && sdi->getSavedValue("2pselected", false)), type);
    }

    static void changeSimplePlayer(SimplePlayer* player, IconType type, bool dual) {
        MoreIconsAPI::updateSimplePlayer(player, MoreIconsAPI::activeForType(type, dual), type);
    }

    static std::unordered_map<std::string, TexturePack>& infoForType(IconType type) {
        switch (type) {
            case IconType::Cube: return ICON_INFO;
            case IconType::Ship: return SHIP_INFO;
            case IconType::Ball: return BALL_INFO;
            case IconType::Ufo: return UFO_INFO;
            case IconType::Wave: return WAVE_INFO;
            case IconType::Robot: return ROBOT_INFO;
            case IconType::Spider: return SPIDER_INFO;
            case IconType::Swing: return SWING_INFO;
            case IconType::Jetpack: return JETPACK_INFO;
            default: {
                static std::unordered_map<std::string, TexturePack> empty;
                return empty;
            }
        }
    }

    static std::string getFrameName(const std::string& name, const std::string& prefix, IconType type) {
        if (type != IconType::Robot && type != IconType::Spider) {
            if (name.ends_with("_2_001.png")) return fmt::format("{}_2_001.png"_spr, prefix);
            else if (name.ends_with("_3_001.png")) return fmt::format("{}_3_001.png"_spr, prefix);
            else if (name.ends_with("_extra_001.png")) return fmt::format("{}_extra_001.png"_spr, prefix);
            else if (name.ends_with("_glow_001.png")) return fmt::format("{}_glow_001.png"_spr, prefix);
            else if (name.ends_with("_001.png")) return fmt::format("{}_001.png"_spr, prefix);
        }
        else for (int i = 1; i < 5; i++) {
            if (name.ends_with(fmt::format("_{:02}_2_001.png", i))) return fmt::format("{}_{:02}_2_001.png"_spr, prefix, i);
            else if (i == 1 && name.ends_with(fmt::format("_{:02}_extra_001.png", i))) return fmt::format("{}_{:02}_extra_001.png"_spr, prefix, i);
            else if (name.ends_with(fmt::format("_{:02}_glow_001.png", i))) return fmt::format("{}_{:02}_glow_001.png"_spr, prefix, i);
            else if (name.ends_with(fmt::format("_{:02}_001.png", i))) return fmt::format("{}_{:02}_001.png"_spr, prefix, i);
        }

        return name;
    }

    static SimplePlayer* findPlayer(cocos2d::CCNode* node);

    static void showInfoPopup(bool folderButton = false);
};
