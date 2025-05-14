#pragma once
#include "IconInfo.hpp"
#include <Geode/binding/GJRobotSprite.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <Geode/loader/Mod.hpp>

#define MORE_ICONS_ID "hiimjustin000.more_icons"
#define MORE_ICONS_EXPAND(x) MORE_ICONS_ID "/" x
#define MORE_ICONS_SIMPLE_PLAYER(...) MoreIcons::SimplePlayerEvent(MORE_ICONS_EXPAND("simple-player"), __VA_ARGS__).post()
#define MORE_ICONS_ROBOT_SPRITE(...) MoreIcons::RobotSpriteEvent(MORE_ICONS_EXPAND("robot-sprite"), __VA_ARGS__).post()
#define MORE_ICONS_PLAYER_OBJECT(...) MoreIcons::PlayerObjectEvent(MORE_ICONS_EXPAND("player-object"), __VA_ARGS__).post()
#define MORE_ICONS_ALL_ICONS(...) MoreIcons::AllIconsEvent(MORE_ICONS_EXPAND("all-icons"), __VA_ARGS__).post()
#define MORE_ICONS_GET_ICONS(...) MoreIcons::GetIconsEvent(MORE_ICONS_EXPAND("get-icons"), __VA_ARGS__).post()
#define MORE_ICONS_GET_ICON(...) MoreIcons::GetIconEvent(MORE_ICONS_EXPAND("get-icon"), __VA_ARGS__).post()
#define MORE_ICONS_LOAD_ICON(...) MoreIcons::LoadIconEvent(MORE_ICONS_EXPAND("load-icon"), __VA_ARGS__).post()
#define MORE_ICONS_UNLOAD_ICON(...) MoreIcons::UnloadIconEvent(MORE_ICONS_EXPAND("unload-icon"), __VA_ARGS__).post()
#define MORE_ICONS_UNLOAD_ICONS(...) MoreIcons::UnloadIconsEvent(MORE_ICONS_EXPAND("unload-icons"), __VA_ARGS__).post()

/**
 * A class that provides an API for interacting with the More Icons mod.
 */
class MoreIcons {
public:
    using SimplePlayerFilter = geode::DispatchFilter<SimplePlayer*, std::string, IconType>;
    using RobotSpriteFilter = geode::DispatchFilter<GJRobotSprite*, std::string, IconType>;
    using PlayerObjectFilter = geode::DispatchFilter<PlayerObject*, std::string, IconType>;
    using AllIconsFilter = geode::DispatchFilter<std::vector<IconInfo*>*>;
    using GetIconsFilter = geode::DispatchFilter<std::vector<IconInfo*>*, IconType>;
    using GetIconFilter = geode::DispatchFilter<IconInfo**, std::string, IconType>;
    using LoadIconFilter = geode::DispatchFilter<std::string, IconType, int>;
    using UnloadIconFilter = geode::DispatchFilter<std::string, IconType, int>;
    using UnloadIconsFilter = geode::DispatchFilter<int>;

    using SimplePlayerEvent = SimplePlayerFilter::Event;
    using RobotSpriteEvent = RobotSpriteFilter::Event;
    using PlayerObjectEvent = PlayerObjectFilter::Event;
    using AllIconsEvent = AllIconsFilter::Event;
    using GetIconsEvent = GetIconsFilter::Event;
    using GetIconEvent = GetIconFilter::Event;
    using LoadIconEvent = LoadIconFilter::Event;
    using UnloadIconEvent = UnloadIconFilter::Event;
    using UnloadIconsEvent = UnloadIconsFilter::Event;

    /**
     * Checks if the More Icons mod is loaded.
     * @returns Whether or not the More Icons mod is loaded.
     */
    static bool loaded() {
        return geode::Loader::get()->isModLoaded(MORE_ICONS_ID);
    }

    /**
     * Returns the Mod object for the More Icons mod.
     * @returns The Mod object for the More Icons mod, or nullptr if the mod is not loaded.
     */
    static geode::Mod* get() {
        static auto mod = geode::Loader::get()->getLoadedMod(MORE_ICONS_ID);
        return mod;
    }

    /**
     * Loads a custom icon into the texture cache.
     * @param name The name of the icon to load.
     * @param type The type of icon to load.
     * @param requestID The request ID of the icon to load.
     */
    static void loadIcon(const std::string& name, IconType type, int requestID) {
        if (!loaded()) return;
        MORE_ICONS_LOAD_ICON(name, type, requestID);
    }

    /**
     * Unloads a custom icon from the texture cache.
     * @param name The name of the icon to unload.
     * @param type The type of icon to unload.
     * @param requestID The request ID of the icon to unload.
     */
    static void unloadIcon(const std::string& name, IconType type, int requestID) {
        if (!loaded()) return;
        MORE_ICONS_UNLOAD_ICON(name, type, requestID);
    }

    /**
     * Unloads all custom icons associated with a request ID.
     * @param requestID The request ID of the icons to unload.
     */
    static void unloadIcons(int requestID) {
        if (!loaded()) return;
        MORE_ICONS_UNLOAD_ICONS(requestID);
    }

    /**
     * Changes the icon of a SimplePlayer object to a custom icon.
     * @param player The SimplePlayer object to change the icon of.
     * @param type The type of icon to change to.
     * @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
     */
    static void updateSimplePlayer(SimplePlayer* player, IconType type, bool dual = false) {
        if (!player || !loaded()) return;
        MORE_ICONS_SIMPLE_PLAYER(player, activeIcon(type, dual), type);
    }

    /**
     * Changes the icon of a SimplePlayer object to a custom icon.
     * @param player The SimplePlayer object to change the icon of.
     * @param icon The name of the icon to change to.
     * @param type The type of icon to change to.
     */
    static void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type) {
        if (!player || icon.empty() || !loaded()) return;
        MORE_ICONS_SIMPLE_PLAYER(player, icon, type);
    }

    /**
     * Changes the icon of a GJRobotSprite object to a custom icon.
     * @param sprite The GJRobotSprite object to change the icon of.
     * @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
     */
    static void updateRobotSprite(GJRobotSprite* sprite, bool dual = false) {
        if (!sprite || !loaded()) return;
        MORE_ICONS_ROBOT_SPRITE(sprite, activeIcon(sprite->m_iconType, dual), sprite->m_iconType);
    }

    /**
     * Changes the icon of a GJRobotSprite object to a custom icon.
     * @param sprite The GJRobotSprite object to change the icon of.
     * @param icon The name of the icon to change to.
     */
    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon) {
        if (!sprite || icon.empty() || !loaded()) return;
        MORE_ICONS_ROBOT_SPRITE(sprite, icon, sprite->m_iconType);
    }

    /**
     * Changes the icon of a GJRobotSprite object to a custom icon.
     * @param sprite The GJRobotSprite object to change the icon of.
     * @param type The type of icon to change to.
     * @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
     */
    static void updateRobotSprite(GJRobotSprite* sprite, IconType type, bool dual = false) {
        if (!sprite || !loaded()) return;
        MORE_ICONS_ROBOT_SPRITE(sprite, activeIcon(type, dual), type);
    }

    /**
     * Changes the icon of a GJRobotSprite object to a custom icon.
     * @param sprite The GJRobotSprite object to change the icon of.
     * @param icon The name of the icon to change to.
     * @param type The type of icon to change to.
     */
    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type) {
        if (!sprite || icon.empty() || !loaded()) return;
        MORE_ICONS_ROBOT_SPRITE(sprite, icon, type);
    }

    /**
     * Returns the icon type for the given player object.
     * @param object The player object to get the icon type for.
     * @returns The icon type for the given player object, or IconType::Cube if the object is null.
     */
    static IconType getIconType(PlayerObject* object) {
        if (!object) return IconType::Cube;

        if (object->m_isShip) {
            if (object->m_isPlatformer) return IconType::Jetpack;
            else return IconType::Ship;
        }
        else if (object->m_isBall) return IconType::Ball;
        else if (object->m_isBird) return IconType::Ufo;
        else if (object->m_isDart) return IconType::Wave;
        else if (object->m_isRobot) return IconType::Robot;
        else if (object->m_isSpider) return IconType::Spider;
        else if (object->m_isSwing) return IconType::Swing;
        else return IconType::Cube;
    }

    /**
     * Changes the icon of a PlayerObject object to a custom icon.
     * @param object The PlayerObject object to change the icon of.
     * @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
     */
    static void updatePlayerObject(PlayerObject* object, bool dual = false) {
        if (!object || !loaded()) return;
        auto type = getIconType(object);
        MORE_ICONS_PLAYER_OBJECT(object, activeIcon(type, dual), type);
    }

    /**
     * Changes the icon of a PlayerObject object to a custom icon.
     * @param object The PlayerObject object to change the icon of.
     * @param icon The name of the icon to change to.
     */
    static void updatePlayerObject(PlayerObject* object, const std::string& icon) {
        if (!object || icon.empty() || !loaded()) return;
        MORE_ICONS_PLAYER_OBJECT(object, icon, getIconType(object));
    }

    /**
     * Changes the icon of a PlayerObject object to a custom icon.
     * @param object The PlayerObject object to change the icon of.
     * @param type The type of icon to change to.
     * @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
     */
    static void updatePlayerObject(PlayerObject* object, IconType type, bool dual = false) {
        if (!object || !loaded()) return;
        MORE_ICONS_PLAYER_OBJECT(object, activeIcon(type, dual), type);
    }

    /**
     * Changes the icon of a PlayerObject object to a custom icon.
     * @param object The PlayerObject object to change the icon of.
     * @param icon The name of the icon to change to.
     * @param type The type of icon to change to.
     */
    static void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type) {
        if (!object || icon.empty() || !loaded()) return;
        MORE_ICONS_PLAYER_OBJECT(object, icon, type);
    }

    /**
     * Returns a vector of all custom icons.
     * @returns A vector of all custom icons.
     */
    static std::vector<IconInfo*> getIcons() {
        std::vector<IconInfo*> vec;
        if (!loaded()) return vec;
        MORE_ICONS_ALL_ICONS(&vec);
        return vec;
    }

    /**
     * Returns a vector of all icons for a specific type.
     * @param type The type of icon to get all icons for.
     * @returns A vector of all icons for the specified type.
     */
    static std::vector<IconInfo*> getIcons(IconType type) {
        std::vector<IconInfo*> vec;
        if (!loaded()) return vec;
        MORE_ICONS_GET_ICONS(&vec, type);
        return vec;
    }

    /**
     * Returns the icon info for a specific icon.
     * @param name The name of the icon to get the info for.
     * @param type The type of icon to get the info for.
     * @returns The icon info for the specified icon, or nullptr if the icon is not found.
     */
    static IconInfo* getIcon(const std::string& name, IconType type) {
        IconInfo* info = nullptr;
        if (!loaded()) return info;
        MORE_ICONS_GET_ICON(&info, name, type);
        return info;
    }

    /**
     * Returns the icon info for the active icon of a specific type.
     * @param type The type of icon to get the info for.
     * @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
     * @returns The icon info for the active icon of the specified type, or nullptr if the icon is not found.
     */
    static IconInfo* getIcon(IconType type, bool dual = false) {
        IconInfo* info = nullptr;
        if (!loaded()) return nullptr;
        MORE_ICONS_GET_ICON(&info, activeIcon(type, dual), type);
        return info;
    }

    /**
     * Returns the save key for the given icon type.
     * @param type The type of icon to get the save key for.
     * @param dual Whether or not to use the icon for the dual player.
     * @returns The save key for the given icon type, or an empty string if there is none.
     */
    static std::string_view saveKey(IconType type, bool dual) {
        auto isDual = geode::Loader::get()->isModLoaded("weebify.separate_dual_icons") && dual;
        switch (type) {
            case IconType::Cube: return isDual ? "icon-dual" : "icon";
            case IconType::Ship: return isDual ? "ship-dual" : "ship";
            case IconType::Ball: return isDual ? "ball-dual" : "ball";
            case IconType::Ufo: return isDual ? "ufo-dual" : "ufo";
            case IconType::Wave: return isDual ? "wave-dual" : "wave";
            case IconType::Robot: return isDual ? "robot-dual" : "robot";
            case IconType::Spider: return isDual ? "spider-dual" : "spider";
            case IconType::Swing: return isDual ? "swing-dual" : "swing";
            case IconType::Jetpack: return isDual ? "jetpack-dual" : "jetpack";
            case IconType::Special: return isDual ? "trail-dual" : "trail";
            default: return "";
        }
    }

    /**
     * Returns the active icon for a specific type.
     * @param type The type of icon to get the active icon for.
     * @param dual Whether or not to get the active icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
     * @returns The active icon for the specified type, or an empty string if the icon is not set.
     */
    static std::string activeIcon(IconType type, bool dual = false) {
        if (!loaded()) return "";
        auto savedType = saveKey(type, dual);
        return !savedType.empty() ? get()->getSavedValue<std::string>(savedType, "") : "";
    }

    /**
     * Sets the icon for a specific type.
     * @param icon The name of the icon to set.
     * @param type The type of icon to set.
     * @param dual Whether or not to set the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
     * @returns The name of the previous icon for the specified type, or an empty string if the icon was not set.
     */
    static std::string setIcon(const std::string& icon, IconType type, bool dual = false) {
        if (!loaded()) return "";
        auto savedType = saveKey(type, dual);
        return !savedType.empty() ? get()->setSavedValue(savedType, icon) : "";
    }

    /**
     * Returns the custom icon name of a node. (cocos2d::CCMotionStreak, CCMenuItemSpriteExtra, GJRobotSprite, PlayerObject, SimplePlayer)
     * @param node The node to get the icon name of.
     * @returns The icon name of the specified node, or an empty string if the icon name is not set.
     */
    static std::string getIconName(cocos2d::CCNode* node) {
        if (!node || !loaded()) return "";
        auto userObject = static_cast<cocos2d::CCString*>(node->getUserObject(MORE_ICONS_EXPAND("name")));
        return userObject ? userObject->m_sString : "";
    }
};
