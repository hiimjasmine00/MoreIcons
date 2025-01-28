#pragma once
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/loader/Dispatch.hpp>

class MoreIcons {
    static void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type) {
        geode::DispatchEvent<SimplePlayer*, std::string, IconType>("hiimjustin000.more_icons/simple-player", player, icon, type).post();
    }

    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon) {
        geode::DispatchEvent<GJRobotSprite*, std::string>("hiimjustin000.more_icons/robot-sprite", sprite, icon).post();
    }

    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type) {
        geode::DispatchEvent<GJRobotSprite*, std::string, IconType>("hiimjustin000.more_icons/robot-sprite", sprite, icon, type).post();
    }

    static void updatePlayerObject(PlayerObject* object, const std::string& icon) {
        geode::DispatchEvent<PlayerObject*, std::string>("hiimjustin000.more_icons/player-object", object, icon).post();
    }

    static void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type) {
        geode::DispatchEvent<PlayerObject*, std::string, IconType>("hiimjustin000.more_icons/player-object", object, icon, type).post();
    }

    static std::vector<std::string> vectorForType(IconType type) {
        std::vector<std::string> vec;
        geode::DispatchEvent<std::vector<std::string>*, IconType>("hiimjustin000.more_icons/all-icons", &vec, type).post();
        return vec;
    }

    static std::string activeForType(IconType type, bool dual = false) {
        std::string icon;
        geode::DispatchEvent<std::string*, IconType, bool>("hiimjustin000.more_icons/active-icon", &icon, type, dual).post();
        return icon;
    }
};
