#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <matjson.hpp>

class IconInfo;
namespace geode {
    class TextInput;
}

template <>
struct matjson::Serialize<IconType> {
    static geode::Result<IconType> fromJson(const matjson::Value& val) {
        return val.asInt().map([](intmax_t v) {
            return (IconType)v;
        });
    }
    static matjson::Value toJson(const IconType& val) {
        return matjson::Value((intmax_t)val);
    }
};

namespace MoreIcons {
    extern geode::Mod* separateDualIcons;

    void blendStreak(cocos2d::CCMotionStreak* streak, IconInfo* info);
    geode::Result<std::filesystem::path> createTrash();
    cocos2d::CCSprite* customIcon(IconInfo* info);
    bool dualSelected();
    std::filesystem::path getEditorDir(IconType type);
    std::filesystem::path getIconDir(IconType type);
    std::string getIconName(int id, IconType type);
    void getIconPaths(IconInfo* info, int id, IconType type, std::filesystem::path& png, std::filesystem::path& plist);
    std::filesystem::path getIconStem(std::string_view name, IconType type);
    std::filesystem::path getIconPath(IconInfo* info, int id, IconType type);
    geode::ZStringView getText(CCTextInputNode* input);
    geode::ZStringView getText(geode::TextInput* input);
    void loadFromSave(IconType type);
    void updateGarage(GJGarageLayer* layer = nullptr);
    cocos2d::ccColor3B vanillaColor1(bool dual);
    cocos2d::ccColor3B vanillaColor2(bool dual);
    cocos2d::ccColor3B vanillaColorGlow(bool dual);
    bool vanillaGlow(bool dual);
    int vanillaIcon(IconType type, bool dual);
}
