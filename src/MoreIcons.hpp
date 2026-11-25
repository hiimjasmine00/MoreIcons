#include <asp/fs/Path.hpp>
#include <ccTypes.h>
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/loader/Types.hpp>
#include <Geode/utils/ZStringView.hpp>

class IconInfo;

namespace MoreIcons {
    extern geode::Mod* separateDualIcons;
    extern geode::Mod* customGamemodeColors;

    void blendStreak(cocos2d::CCMotionStreak* streak, IconInfo* info);
    geode::Result<asp::fs::Path> createTrash();
    cocos2d::ccColor3B currentColor1(IconType type, bool dual);
    cocos2d::ccColor3B currentColor2(IconType type, bool dual);
    cocos2d::ccColor3B currentColorGlow(IconType type, bool dual);
    bool currentGlow(bool dual);
    cocos2d::CCSprite* customIcon(IconInfo* info);
    bool dualSelected();
    cocos2d::CCSpriteFrame* frameWithTexture(cocos2d::CCTexture2D* texture);
    asp::fs::Path getEditorDir(IconType type);
    asp::fs::Path getIconDir(IconType type);
    std::string getIconName(int id, IconType type);
    void getIconPaths(IconInfo* info, int id, IconType type, asp::fs::Path& png, asp::fs::Path& plist);
    asp::fs::Path getFirePath(IconInfo* info, int id, int frame);
    asp::fs::Path getIconStem(std::string_view name, IconType type);
    asp::fs::Path getIconPath(IconInfo* info, int id, IconType type);
    void loadFromSave(IconType type);
    void setupCustomStreak(PlayerObject* player);
    void setTexture(cocos2d::CCSprite* sprite, cocos2d::CCTexture2D* texture);
    void updateGarageAndNotify(geode::ZStringView message);
    int vanillaIcon(IconType type, bool dual);
}
