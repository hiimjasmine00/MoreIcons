#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/loader/Types.hpp>

class IconInfo;

namespace MoreIcons {
    extern geode::Mod* separateDualIcons;
    extern geode::Mod* customGamemodeColors;

    void blendStreak(cocos2d::CCMotionStreak* streak, IconInfo* info);
    geode::Result<std::filesystem::path> createTrash();
    cocos2d::ccColor3B currentColor1(IconType type, bool dual);
    cocos2d::ccColor3B currentColor2(IconType type, bool dual);
    cocos2d::ccColor3B currentColorGlow(IconType type, bool dual);
    bool currentGlow(bool dual);
    cocos2d::CCSprite* customIcon(IconInfo* info);
    bool dualSelected();
    cocos2d::CCSpriteFrame* frameWithTexture(cocos2d::CCTexture2D* texture);
    std::filesystem::path getEditorDir(IconType type);
    std::filesystem::path getIconDir(IconType type);
    std::string getIconName(int id, IconType type);
    void getIconPaths(IconInfo* info, int id, IconType type, std::filesystem::path& png, std::filesystem::path& plist);
    std::filesystem::path getFirePath(IconInfo* info, int id, int frame);
    std::filesystem::path getFirePath(IconInfo* info, int id, std::string_view frameSuffix);
    std::filesystem::path getIconStem(std::string_view name, IconType type);
    std::filesystem::path getIconPath(IconInfo* info, int id, IconType type);
    void loadFromSave(IconType type);
    void setupCustomStreak(PlayerObject* player);
    void setTexture(cocos2d::CCSprite* sprite, cocos2d::CCTexture2D* texture);
    void updateGarageAndNotify(geode::ZStringView message);
    int vanillaIcon(IconType type, bool dual);
}
