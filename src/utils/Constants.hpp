#include <Geode/Enums.hpp>
#include <Geode/loader/Types.hpp>

namespace Constants {
    const char* getSeverityFrame(geode::Severity severity);
    std::string_view getSingularLowercase(IconType type);
    std::string_view getPluralLowercase(IconType type);
    std::string_view getSingularUppercase(IconType type);
    std::string_view getPluralUppercase(IconType type);
    #ifdef GEODE_IS_WINDOWS
    std::wstring_view getFolderName(IconType type);
    #else
    std::string_view getFolderName(IconType type);
    #endif
    float getIconGap(IconType type);
    UnlockType getUnlockType(IconType type);
}
