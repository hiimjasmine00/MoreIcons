#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/Result.hpp>

class IconInfo;

class MoreIcons {
public:
    static geode::Mod* separateDualIcons;

    static void blendStreak(cocos2d::CCMotionStreak* streak, IconInfo* info);
    static geode::Result<std::filesystem::path> createTrash();
    static cocos2d::CCSprite* customIcon(IconInfo* info);
    static bool dualSelected();
    static std::filesystem::path getEditorDir(IconType type);
    static std::filesystem::path getIconDir(IconType type);
    static std::string getIconName(int id, IconType type);
    static std::pair<std::string, std::string> getIconPaths(int id, IconType type);
    static std::filesystem::path getIconStem(std::string_view name, IconType type);
    static std::string getTrailTexture(int id);
    static void updateGarage(GJGarageLayer* layer = nullptr);
    static cocos2d::ccColor3B vanillaColor1(bool dual);
    static cocos2d::ccColor3B vanillaColor2(bool dual);
    static cocos2d::ccColor3B vanillaColorGlow(bool dual);
    static bool vanillaGlow(bool dual);
    static int vanillaIcon(IconType type, bool dual);
};
