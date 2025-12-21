#include "Constants.hpp"

using namespace geode::prelude;
using namespace std::string_literals;
using namespace std::string_view_literals;

const char* Constants::getSeverityFrame(Severity severity) {
    switch (severity) {
        case Severity::Info: return "GJ_infoIcon_001.png";
        case Severity::Warning: return "geode.loader/info-warning.png";
        case Severity::Error: return "geode.loader/info-alert.png";
        default: return "cc_2x2_white_image";
    }
}

std::string_view Constants::getIconLabel(IconType type, bool uppercase, bool plural) {
    switch (type) {
        case IconType::Cube:
            return uppercase ? (plural ? "Icons"sv : "Icon"sv) : (plural ? "icons"sv : "icon"sv);
        case IconType::Ship:
            return uppercase ? (plural ? "Ships"sv : "Ship"sv) : (plural ? "ships"sv : "ship"sv);
        case IconType::Ball:
            return uppercase ? (plural ? "Balls"sv : "Ball"sv) : (plural ? "balls"sv : "ball"sv);
        case IconType::Ufo:
            return plural ? "UFOs"sv : "UFO"sv;
        case IconType::Wave:
            return uppercase ? (plural ? "Waves"sv : "Wave"sv) : (plural ? "waves"sv : "wave"sv);
        case IconType::Robot:
            return uppercase ? (plural ? "Robots"sv : "Robot"sv) : (plural ? "robots"sv : "robot"sv);
        case IconType::Spider:
            return uppercase ? (plural ? "Spiders"sv : "Spider"sv) : (plural ? "spiders"sv : "spider"sv);
        case IconType::Swing:
            return uppercase ? (plural ? "Swings"sv : "Swing"sv) : (plural ? "swings"sv : "swing"sv);
        case IconType::Jetpack:
            return uppercase ? (plural ? "Jetpacks"sv : "Jetpack"sv) : (plural ? "jetpacks"sv : "jetpack"sv);
        case IconType::DeathEffect:
            return uppercase ? (plural ? "Death Effects"sv : "Death Effect"sv) : (plural ? "death effects"sv : "death effect"sv);
        case IconType::Special:
            return uppercase ? (plural ? "Trails"sv : "Trail"sv) : (plural ? "trails"sv : "trail"sv);
        case IconType::ShipFire:
            return uppercase ? (plural ? "Ship Fires"sv : "Ship Fire"sv) : (plural ? "ship fires"sv : "ship fire"sv);
        default:
            return ""sv;
    }
}

#ifdef GEODE_IS_WINDOWS
#define L(x) L##x
#else
#define L(x) x
#endif

std::filesystem::path Constants::getFolderName(IconType type) {
    switch (type) {
        case IconType::Cube: return L("icon"s);
        case IconType::Ship: return L("ship"s);
        case IconType::Ball: return L("ball"s);
        case IconType::Ufo: return L("ufo"s);
        case IconType::Wave: return L("wave"s);
        case IconType::Robot: return L("robot"s);
        case IconType::Spider: return L("spider"s);
        case IconType::Swing: return L("swing"s);
        case IconType::Jetpack: return L("jetpack"s);
        case IconType::DeathEffect: return L("death"s);
        case IconType::Special: return L("trail"s);
        case IconType::ShipFire: return L("fire"s);
        default: return {};
    }
}
