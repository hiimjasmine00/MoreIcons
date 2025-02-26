#pragma once
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/loader/Dispatch.hpp>

#define MORE_ICONS_ID "hiimjustin000.more_icons"
#define MORE_ICONS_EXPAND(x) MORE_ICONS_ID "/" x

class MoreIcons {
public:
    static void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type) {
        geode::DispatchEvent<SimplePlayer*, std::string, IconType>(MORE_ICONS_EXPAND("simple-player"), player, icon, type).post();
    }

    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon) {
        geode::DispatchEvent<GJRobotSprite*, std::string>(MORE_ICONS_EXPAND("robot-sprite"), sprite, icon).post();
    }

    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type) {
        geode::DispatchEvent<GJRobotSprite*, std::string, IconType>(MORE_ICONS_EXPAND("robot-sprite"), sprite, icon, type).post();
    }

    static void updatePlayerObject(PlayerObject* object, const std::string& icon) {
        geode::DispatchEvent<PlayerObject*, std::string>(MORE_ICONS_EXPAND("player-object"), object, icon).post();
    }

    static void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type) {
        geode::DispatchEvent<PlayerObject*, std::string, IconType>(MORE_ICONS_EXPAND("player-object"), object, icon, type).post();
    }

    static std::vector<std::string> vectorForType(IconType type) {
        std::vector<std::string> vec;
        geode::DispatchEvent<std::vector<std::string>*, IconType>(MORE_ICONS_EXPAND("all-icons"), &vec, type).post();
        return vec;
    }

    static std::string activeForType(IconType type, bool dual = false) {
        std::string icon;
        geode::DispatchEvent<std::string*, IconType, bool>(MORE_ICONS_EXPAND("active-icon"), &icon, type, dual).post();
        return icon;
    }

    static void setIcon(const std::string& icon, IconType type, bool dual = false) {
        geode::DispatchEvent<std::string, IconType, bool>(MORE_ICONS_EXPAND("set-icon"), icon, type, dual).post();
    }
};
