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
    if (uppercase) {
        if (plural) {
            switch (type) {
                case IconType::Cube: return "Icons";
                case IconType::Ship: return "Ships";
                case IconType::Ball: return "Balls";
                case IconType::Ufo: return "UFOs";
                case IconType::Wave: return "Waves";
                case IconType::Robot: return "Robots";
                case IconType::Spider: return "Spiders";
                case IconType::Swing: return "Swings";
                case IconType::Jetpack: return "Jetpacks";
                case IconType::DeathEffect: return "Death Effects";
                case IconType::Special: return "Trails";
                case IconType::ShipFire: return "Ship Fires";
                default: return "";
            }
        }
        else {
            switch (type) {
                case IconType::Cube: return "Icon";
                case IconType::Ship: return "Ship";
                case IconType::Ball: return "Ball";
                case IconType::Ufo: return "UFO";
                case IconType::Wave: return "Wave";
                case IconType::Robot: return "Robot";
                case IconType::Spider: return "Spider";
                case IconType::Swing: return "Swing";
                case IconType::Jetpack: return "Jetpack";
                case IconType::DeathEffect: return "Death Effect";
                case IconType::Special: return "Trail";
                case IconType::ShipFire: return "Ship Fire";
                default: return "";
            }
        }
    }
    else {
        if (plural) {
            switch (type) {
                case IconType::Cube: return "icons";
                case IconType::Ship: return "ships";
                case IconType::Ball: return "balls";
                case IconType::Ufo: return "UFOs";
                case IconType::Wave: return "waves";
                case IconType::Robot: return "robots";
                case IconType::Spider: return "spiders";
                case IconType::Swing: return "swings";
                case IconType::Jetpack: return "jetpacks";
                case IconType::DeathEffect: return "death effects";
                case IconType::Special: return "trails";
                case IconType::ShipFire: return "ship fires";
                default: return "";
            }
        }
        else {
            switch (type) {
                case IconType::Cube: return "icon";
                case IconType::Ship: return "ship";
                case IconType::Ball: return "ball";
                case IconType::Ufo: return "UFO";
                case IconType::Wave: return "wave";
                case IconType::Robot: return "robot";
                case IconType::Spider: return "spider";
                case IconType::Swing: return "swing";
                case IconType::Jetpack: return "jetpack";
                case IconType::DeathEffect: return "death effect";
                case IconType::Special: return "trail";
                case IconType::ShipFire: return "ship fire";
                default: return "";
            }
        }
    }
}

#ifdef GEODE_IS_WINDOWS
std::wstring_view Constants::getFolderName(IconType type) {
    switch (type) {
        case IconType::Cube: return L"icon";
        case IconType::Ship: return L"ship";
        case IconType::Ball: return L"ball";
        case IconType::Ufo: return L"ufo";
        case IconType::Wave: return L"wave";
        case IconType::Robot: return L"robot";
        case IconType::Spider: return L"spider";
        case IconType::Swing: return L"swing";
        case IconType::Jetpack: return L"jetpack";
        case IconType::DeathEffect: return L"death";
        case IconType::Special: return L"trail";
        case IconType::ShipFire: return L"fire";
        default: return L"";
    }
}
#else
std::string_view Constants::getFolderName(IconType type) {
    switch (type) {
        case IconType::Cube: return "icon";
        case IconType::Ship: return "ship";
        case IconType::Ball: return "ball";
        case IconType::Ufo: return "ufo";
        case IconType::Wave: return "wave";
        case IconType::Robot: return "robot";
        case IconType::Spider: return "spider";
        case IconType::Swing: return "swing";
        case IconType::Jetpack: return "jetpack";
        case IconType::DeathEffect: return "death";
        case IconType::Special: return "trail";
        case IconType::ShipFire: return "fire";
        default: return "";
    }
}
#endif
