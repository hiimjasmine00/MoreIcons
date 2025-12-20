#include "Constants.hpp"

using namespace geode::prelude;

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
        case IconType::Cube: return uppercase ? (plural ? "Icons" : "Icon") : (plural ? "icons" : "icon");
        case IconType::Ship: return uppercase ? (plural ? "Ships" : "Ship") : (plural ? "ships" : "ship");
        case IconType::Ball: return uppercase ? (plural ? "Balls" : "Ball") : (plural ? "balls" : "ball");
        case IconType::Ufo: return plural ? "UFOs" : "UFO";
        case IconType::Wave: return uppercase ? (plural ? "Waves" : "Wave") : (plural ? "waves" : "wave");
        case IconType::Robot: return uppercase ? (plural ? "Robots" : "Robot") : (plural ? "robots" : "robot");
        case IconType::Spider: return uppercase ? (plural ? "Spiders" : "Spider") : (plural ? "spiders" : "spider");
        case IconType::Swing: return uppercase ? (plural ? "Swings" : "Swing") : (plural ? "swings" : "swing");
        case IconType::Jetpack: return uppercase ? (plural ? "Jetpacks" : "Jetpack") : (plural ? "jetpacks" : "jetpack");
        case IconType::DeathEffect: return uppercase ? (plural ? "Death Effects" : "Death Effect") : (plural ? "death effects" : "death effect");
        case IconType::Special: return uppercase ? (plural ? "Trails" : "Trail") : (plural ? "trails" : "trail");
        case IconType::ShipFire: return uppercase ? (plural ? "Ship Fires" : "Ship Fire") : (plural ? "ship fires" : "ship fire");
        default: return "";
    }
}

#ifdef GEODE_IS_WINDOWS
#define L(x) L##x
#else
#define L(x) x
#endif

std::filesystem::path Constants::getFolderName(IconType type) {
    switch (type) {
        case IconType::Cube: return L("icon");
        case IconType::Ship: return L("ship");
        case IconType::Ball: return L("ball");
        case IconType::Ufo: return L("ufo");
        case IconType::Wave: return L("wave");
        case IconType::Robot: return L("robot");
        case IconType::Spider: return L("spider");
        case IconType::Swing: return L("swing");
        case IconType::Jetpack: return L("jetpack");
        case IconType::DeathEffect: return L("death");
        case IconType::Special: return L("trail");
        case IconType::ShipFire: return L("fire");
        default: return {};
    }
}
