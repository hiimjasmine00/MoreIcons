#pragma once
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

/**
 * A class that provides an API for interacting with the More Icons mod.
 */
class MoreIcons {
public:
    using SimplePlayerEvent = geode::DispatchEvent<SimplePlayer*, std::string, IconType>;
    using SimplePlayerFilter = geode::DispatchFilter<SimplePlayer*, std::string, IconType>;
    using RobotSpriteEvent = geode::DispatchEvent<GJRobotSprite*, std::string, IconType>;
    using RobotSpriteFilter = geode::DispatchFilter<GJRobotSprite*, std::string, IconType>;
    using PlayerObjectEvent = geode::DispatchEvent<PlayerObject*, std::string, IconType>;
    using PlayerObjectFilter = geode::DispatchFilter<PlayerObject*, std::string, IconType>;
    using AllIconsEvent = geode::DispatchEvent<std::vector<std::string>*, IconType>;
    using AllIconsFilter = geode::DispatchFilter<std::vector<std::string>*, IconType>;

    /**
     * Checks if the More Icons mod is loaded.
     * @returns Whether or not the More Icons mod is loaded.
     */
    static bool loaded() {
        static auto loaded = geode::Loader::get()->isModLoaded(MORE_ICONS_ID);
        return loaded;
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
     * Changes the icon of a SimplePlayer object to a custom icon.
     * @param player The SimplePlayer object to change the icon of.
     * @param type The type of icon to change to.
     * @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
     */
    static void updateSimplePlayer(SimplePlayer* player, IconType type, bool dual = false) {
        if (!player || !loaded()) return;
        MORE_ICONS_SIMPLE_PLAYER(player, activeForType(type, dual), type);
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
        MORE_ICONS_ROBOT_SPRITE(sprite, activeForType(sprite->m_iconType, dual), sprite->m_iconType);
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
        MORE_ICONS_ROBOT_SPRITE(sprite, activeForType(type, dual), type);
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
        MORE_ICONS_PLAYER_OBJECT(object, activeForType(type, dual), type);
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
        MORE_ICONS_PLAYER_OBJECT(object, activeForType(type, dual), type);
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
     * Returns a vector of all icons for a specific type.
     * @param type The type of icon to get all icons for.
     * @returns A vector of all icons for the specified type.
     */
    static std::vector<std::string> vectorForType(IconType type) {
        std::vector<std::string> vec;
        if (!loaded()) return vec;
        MORE_ICONS_ALL_ICONS(&vec, type);
        return vec;
    }

    /**
     * Returns the save key for the given icon type.
     * @param type The type of icon to get the save key for.
     * @param dual Whether or not to use the icon for the dual player.
     * @returns The save key for the given icon type, or an empty string if there is none.
     */
    static std::string_view savedForType(IconType type, bool dual) {
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
    static std::string activeForType(IconType type, bool dual = false) {
        if (!loaded()) return "";
        auto savedType = savedForType(type, dual);
        return !savedType.empty() ? get()->getSavedValue<std::string>(savedType, "") : "";
    }

    /**
     * Sets the icon for a specific type.
     * @param icon The name of the icon to set.
     * @param type The type of icon to set.
     * @param dual Whether or not to set the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
     */
    static void setIcon(const std::string& icon, IconType type, bool dual = false) {
        if (!loaded()) return;
        auto savedType = savedForType(type, dual);
        if (!savedType.empty()) get()->setSavedValue(savedType, icon);
    }

    /**
     * Return the custom icon name of a node. (cocos2d::CCMotionStreak, GJRobotSprite, PlayerObject, SimplePlayer)
     * @param node The node to get the icon name of.
     * @returns The icon name of the specified node, or an empty string if the icon name is not set.
     */
    static std::string getIconName(cocos2d::CCNode* node) {
        if (!node || !loaded()) return "";
        auto userObject = static_cast<cocos2d::CCString*>(node->getUserObject(MORE_ICONS_EXPAND("name")));
        return userObject ? userObject->getCString() : "";
    }
};
