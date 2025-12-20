#include <filesystem>
#include <Geode/Enums.hpp>
#include <Geode/loader/Types.hpp>

class Constants {
public:
    static const char* getSeverityFrame(geode::Severity severity);
    static std::string_view getIconLabel(IconType type, bool uppercase, bool plural);
    static std::filesystem::path getFolderName(IconType type);
};
