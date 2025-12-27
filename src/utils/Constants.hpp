#include <Geode/Enums.hpp>
#include <Geode/loader/Types.hpp>

class Constants {
public:
    static const char* getSeverityFrame(geode::Severity severity);
    static std::string_view getSingularLowercase(IconType type);
    static std::string_view getPluralLowercase(IconType type);
    static std::string_view getSingularUppercase(IconType type);
    static std::string_view getPluralUppercase(IconType type);
    #ifdef GEODE_IS_WINDOWS
    static std::wstring_view getFolderName(IconType type);
    #else
    static std::string_view getFolderName(IconType type);
    #endif
};
