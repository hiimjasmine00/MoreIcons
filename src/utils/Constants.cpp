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

std::string_view Constants::getSingularLowercase(IconType type) {
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

std::string_view Constants::getPluralLowercase(IconType type) {
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

std::string_view Constants::getSingularUppercase(IconType type) {
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

std::string_view Constants::getPluralUppercase(IconType type) {
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

float Constants::getIconGap(IconType type) {
    switch (type) {
        case IconType::Cube:
        case IconType::Wave:
        case IconType::DeathEffect:
        case IconType::Special:
        case IconType::ShipFire:
            return 9.0f;
        case IconType::Ship:
        case IconType::Jetpack:
            return 13.0f;
        case IconType::Ball:
            return 10.0f;
        case IconType::Ufo:
        case IconType::Swing:
            return 11.0f;
        case IconType::Robot:
        case IconType::Spider:
            return 12.0f;
        default:
            return 8.0f;
    }
}

UnlockType Constants::getUnlockType(IconType type) {
    switch (type) {
        case IconType::Cube: return UnlockType::Cube;
        case IconType::Ship: return UnlockType::Ship;
        case IconType::Ball: return UnlockType::Ball;
        case IconType::Ufo: return UnlockType::Bird;
        case IconType::Wave: return UnlockType::Dart;
        case IconType::Robot: return UnlockType::Robot;
        case IconType::Spider: return UnlockType::Spider;
        case IconType::Swing: return UnlockType::Swing;
        case IconType::Jetpack: return UnlockType::Jetpack;
        case IconType::DeathEffect: return UnlockType::Death;
        case IconType::Special: return UnlockType::Streak;
        case IconType::ShipFire: return UnlockType::ShipFire;
        default: return (UnlockType)0;
    }
}
