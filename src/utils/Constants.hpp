#include <ccTypes.h>
#include <Geode/Enums.hpp>
#include <Geode/loader/Types.hpp>

namespace Constants {
    const char* getSeverityFrame(geode::Severity severity);
    std::string_view getSingularLowercase(IconType type);
    std::string_view getPluralLowercase(IconType type);
    std::string_view getSingularUppercase(IconType type);
    std::string_view getPluralUppercase(IconType type);
    std::string_view getFolderName(IconType type);
    float getIconGap(IconType type);
    UnlockType getUnlockType(IconType type);
    cocos2d::ccColor3B getColor(int index);
}
