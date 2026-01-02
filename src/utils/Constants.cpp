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

ccColor3B Constants::getColor(int index) {
    switch (index) {
        case 0: return { 125, 255, 0 };
        case 1: return { 0, 255, 0 };
        case 2: return { 0, 255, 125 };
        case 3: return { 0, 255, 255 };
        case 4: return { 0, 125, 255 };
        case 5: return { 0, 0, 255 };
        case 6: return { 125, 0, 255 };
        case 7: return { 255, 0, 255 };
        case 8: return { 255, 0, 125 };
        case 9: return { 255, 0, 0 };
        case 10: return { 255, 125, 0 };
        case 11: return { 255, 255, 0 };
        case 12: return { 255, 255, 255 };
        case 13: return { 185, 0, 255 };
        case 14: return { 255, 185, 0 };
        case 15: return { 0, 0, 0 };
        case 16: return { 0, 200, 255 };
        case 17: return { 175, 175, 175 };
        case 18: return { 90, 90, 90 };
        case 19: return { 255, 125, 125 };
        case 20: return { 0, 175, 75 };
        case 21: return { 0, 125, 125 };
        case 22: return { 0, 75, 175 };
        case 23: return { 75, 0, 175 };
        case 24: return { 125, 0, 125 };
        case 25: return { 175, 0, 75 };
        case 26: return { 175, 75, 0 };
        case 27: return { 125, 125, 0 };
        case 28: return { 75, 175, 0 };
        case 29: return { 255, 75, 0 };
        case 30: return { 150, 50, 0 };
        case 31: return { 150, 100, 0 };
        case 32: return { 100, 150, 0 };
        case 33: return { 0, 150, 100 };
        case 34: return { 0, 100, 150 };
        case 35: return { 100, 0, 150 };
        case 36: return { 150, 0, 100 };
        case 37: return { 150, 0, 0 };
        case 38: return { 0, 150, 0 };
        case 39: return { 0, 0, 150 };
        case 40: return { 125, 255, 175 };
        case 41: return { 125, 125, 255 };
        case 42: return { 255, 250, 127 };
        case 43: return { 250, 127, 255 };
        case 44: return { 0, 255, 192 };
        case 45: return { 80, 50, 14 };
        case 46: return { 205, 165, 118 };
        case 47: return { 182, 128, 255 };
        case 48: return { 255, 58, 58 };
        case 49: return { 77, 77, 143 };
        case 50: return { 0, 10, 76 };
        case 51: return { 253, 212, 206 };
        case 52: return { 190, 181, 255 };
        case 53: return { 112, 0, 0 };
        case 54: return { 82, 2, 0 };
        case 55: return { 56, 1, 6 };
        case 56: return { 128, 79, 79 };
        case 57: return { 122, 53, 53 };
        case 58: return { 81, 36, 36 };
        case 59: return { 163, 98, 70 };
        case 60: return { 117, 73, 54 };
        case 61: return { 86, 53, 40 };
        case 62: return { 255, 185, 114 };
        case 63: return { 255, 160, 64 };
        case 64: return { 102, 49, 30 };
        case 65: return { 91, 39, 0 };
        case 66: return { 71, 32, 0 };
        case 67: return { 167, 123, 77 };
        case 68: return { 109, 83, 57 };
        case 69: return { 81, 62, 42 };
        case 70: return { 255, 255, 192 };
        case 71: return { 253, 224, 160 };
        case 72: return { 192, 255, 160 };
        case 73: return { 177, 255, 109 };
        case 74: return { 192, 255, 224 };
        case 75: return { 148, 255, 228 };
        case 76: return { 67, 161, 138 };
        case 77: return { 49, 109, 95 };
        case 78: return { 38, 84, 73 };
        case 79: return { 0, 96, 0 };
        case 80: return { 0, 64, 0 };
        case 81: return { 0, 96, 96 };
        case 82: return { 0, 64, 64 };
        case 83: return { 160, 255, 255 };
        case 84: return { 1, 7, 112 };
        case 85: return { 0, 73, 109 };
        case 86: return { 0, 50, 76 };
        case 87: return { 0, 38, 56 };
        case 88: return { 80, 128, 173 };
        case 89: return { 51, 83, 117 };
        case 90: return { 35, 60, 86 };
        case 91: return { 224, 224, 224 };
        case 92: return { 61, 6, 140 };
        case 93: return { 55, 8, 96 };
        case 94: return { 64, 64, 64 };
        case 95: return { 111, 73, 164 };
        case 96: return { 84, 54, 127 };
        case 97: return { 66, 42, 99 };
        case 98: return { 252, 181, 255 };
        case 99: return { 175, 87, 175 };
        case 100: return { 130, 67, 130 };
        case 101: return { 94, 49, 94 };
        case 102: return { 128, 128, 128 };
        case 103: return { 102, 3, 62 };
        case 104: return { 71, 1, 52 };
        case 105: return { 210, 255, 50 };
        case 106: return { 118, 189, 255 };
        default: return { 255, 255, 255 };
    }
}
