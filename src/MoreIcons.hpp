#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/Result.hpp>

class IconInfo;

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
    void updateGarage(GJGarageLayer* layer = nullptr);
    cocos2d::ccColor3B vanillaColor1(bool dual);
    cocos2d::ccColor3B vanillaColor2(bool dual);
    cocos2d::ccColor3B vanillaColorGlow(bool dual);
    bool vanillaGlow(bool dual);
    int vanillaIcon(IconType type, bool dual);
}
