#pragma once
#include "IconInfo.hpp"
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GJRobotSprite.hpp>
#include <Geode/binding/PlayerObject.hpp>
#if defined(MORE_ICONS_EVENTS) || defined(GEODE_DEFINE_EVENT_EXPORTS)
#include <Geode/loader/Dispatch.hpp>
#endif
#ifdef GEODE_DEFINE_EVENT_EXPORTS
#define MI_INLINE
#else
#define MI_INLINE inline
#endif
#include <Geode/loader/Mod.hpp>

#ifdef MY_MOD_ID
#undef MY_MOD_ID
#endif
#define MY_MOD_ID "hiimjustin000.more_icons"

namespace more_icons {
    static constexpr std::string_view ID = MY_MOD_ID;
    namespace internal {
        template <size_t N>
        struct BoyIfYouDontGet {
            static constexpr size_t extra = sizeof(MY_MOD_ID);
            char buffer[extra + N]{};
            constexpr BoyIfYouDontGet(const char (&pp)[N]) {
                char id[] = MY_MOD_ID;
                for (int i = 0; i < sizeof(id); i++) {
                    buffer[i] = id[i];
                }
                buffer[extra - 1] = '/';
                for (int i = 0; i < N; i++) {
                    buffer[extra + i] = pp[i];
                }
            }
        };
    }
}

template <more_icons::internal::BoyIfYouDontGet Str>
constexpr auto operator""_mi() {
    return Str.buffer;
}

#ifndef MORE_ICONS_EVENTS
    #ifdef GEODE_IS_WINDOWS
        #ifdef MORE_ICONS_EXPORTING
            #define MORE_ICONS_DLL __declspec(dllexport)
        #else
            #define MORE_ICONS_DLL __declspec(dllimport)
        #endif
    #else
        #ifdef MORE_ICONS_EXPORTING
            #define MORE_ICONS_DLL __attribute__((visibility("default")))
        #else
            #define MORE_ICONS_DLL
        #endif
    #endif
#else
    #define MORE_ICONS_DLL
#endif

namespace more_icons {
    /// Checks if the More Icons mod is loaded.
    /// @returns Whether or not the More Icons mod is loaded.
    inline bool loaded() {
        return geode::Loader::get()->isModLoaded({ ID.data(), ID.size() });
    }

    /// Returns the Mod object for the More Icons mod.
    /// @returns The Mod object for the More Icons mod, or nullptr if the mod is not loaded.
    inline geode::Mod* get() {
        return geode::Loader::get()->getLoadedMod({ ID.data(), ID.size() });
    }

    /// Returns the save key for the given icon type.
    /// @param type The type of icon to get the save key for.
    /// @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
    /// @returns The save key for the given icon type, or an empty string if there is none.
    inline std::string_view saveKey(IconType type, bool dual = false) {
        auto isDual = dual && geode::Loader::get()->isModLoaded("weebify.separate_dual_icons");
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

    /// Returns the active icon for a specific type.
    /// @param type The type of icon to get the active icon for.
    /// @param dual Whether or not to get the active icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
    /// @returns The active icon for the specified type, or an empty string if the icon is not set.
    inline std::string activeIcon(IconType type, bool dual = false) {
        auto mod = get();
        if (!mod) return {};
        auto savedType = saveKey(type, dual);
        return !savedType.empty() ? mod->getSavedValue<std::string>(savedType, {}) : std::string();
    }

    /// Sets the icon for a specific type.
    /// @param icon The name of the icon to set.
    /// @param type The type of icon to set.
    /// @param dual Whether or not to set the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
    /// @returns The name of the previous icon for the specified type, or an empty string if the icon was not set.
    inline std::string setIcon(const std::string& icon, IconType type, bool dual = false) {
        auto mod = get();
        if (!mod) return {};
        auto savedType = saveKey(type, dual);
        return !savedType.empty() ? mod->setSavedValue(savedType, icon) : std::string();
    }

    /// Loads a custom icon into the texture cache.
    /// @param name The name of the icon to load.
    /// @param type The type of icon to load.
    /// @param requestID The request ID of the icon to load.
    MORE_ICONS_DLL cocos2d::CCTexture2D* loadIcon(const std::string& name, IconType type, int requestID);

    /// Unloads a custom icon from the texture cache.
    /// @param name The name of the icon to unload.
    /// @param type The type of icon to unload.
    /// @param requestID The request ID of the icon to unload.
    MORE_ICONS_DLL void unloadIcon(const std::string& name, IconType type, int requestID);

    /// Unloads all custom icons associated with a request ID.
    /// @param requestID The request ID of the icons to unload.
    MORE_ICONS_DLL void unloadIcons(int requestID);

    /// Changes the icon of a SimplePlayer object to a custom icon.
    /// @param player The SimplePlayer object to change the icon of.
    /// @param icon The name of the icon to change to.
    /// @param type The type of icon to change to.
    MORE_ICONS_DLL void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type);

    /// Changes the icon of a SimplePlayer object to a custom icon.
    /// @param player The SimplePlayer object to change the icon of.
    /// @param type The type of icon to change to.
    /// @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
    inline void updateSimplePlayer(SimplePlayer* player, IconType type, bool dual = false) {
        if (player) updateSimplePlayer(player, activeIcon(type, dual), type);
    }

    /// Changes the icon of a GJRobotSprite object to a custom icon.
    /// @param sprite The GJRobotSprite object to change the icon of.
    /// @param icon The name of the icon to change to.
    /// @param type The type of icon to change to.
    MORE_ICONS_DLL void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type);

    /// Changes the icon of a GJRobotSprite object to a custom icon.
    /// @param sprite The GJRobotSprite object to change the icon of.
    /// @param type The type of icon to change to.
    /// @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
    inline void updateRobotSprite(GJRobotSprite* sprite, IconType type, bool dual = false) {
        if (sprite) updateRobotSprite(sprite, activeIcon(type, dual), type);
    }

    /// Changes the icon of a GJRobotSprite object to a custom icon.
    /// @param sprite The GJRobotSprite object to change the icon of.
    /// @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
    inline void updateRobotSprite(GJRobotSprite* sprite, bool dual = false) {
        if (sprite) updateRobotSprite(sprite, sprite->m_iconType, dual);
    }

    /// Changes the icon of a GJRobotSprite object to a custom icon.
    /// @param sprite The GJRobotSprite object to change the icon of.
    /// @param icon The name of the icon to change to.
    inline void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon) {
        if (sprite && !icon.empty()) updateRobotSprite(sprite, icon, sprite->m_iconType);
    }

    /// Returns the icon type for the given player object.
    /// @param object The player object to get the icon type for.
    /// @returns The icon type for the given player object, or IconType::Cube if the object is null.
    inline IconType getIconType(PlayerObject* object) {
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

    /// Changes the icon of a PlayerObject object to a custom icon.
    /// @param object The PlayerObject object to change the icon of.
    /// @param icon The name of the icon to change to.
    /// @param type The type of icon to change to.
    MORE_ICONS_DLL void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type);

    /// Changes the icon of a PlayerObject object to a custom icon.
    /// @param object The PlayerObject object to change the icon of.
    /// @param type The type of icon to change to.
    /// @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
    inline void updatePlayerObject(PlayerObject* object, IconType type, bool dual = false) {
        if (object) updatePlayerObject(object, activeIcon(type, dual), type);
    }

    /// Changes the icon of a PlayerObject object to a custom icon.
    /// @param object The PlayerObject object to change the icon of.
    /// @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
    inline void updatePlayerObject(PlayerObject* object, bool dual = false) {
        if (object) updatePlayerObject(object, getIconType(object), dual);
    }

    /// Changes the icon of a PlayerObject object to a custom icon.
    /// @param object The PlayerObject object to change the icon of.
    /// @param icon The name of the icon to change to.
    inline void updatePlayerObject(PlayerObject* object, const std::string& icon) {
        if (object && !icon.empty()) updatePlayerObject(object, icon, getIconType(object));
    }

    /// Returns a map of all custom icons.
    /// @returns A map of all custom icons.
    MORE_ICONS_DLL std::map<IconType, std::vector<IconInfo>> getIcons();

    /// Returns a vector of all icons for a specific type.
    /// @param type The type of icon to get all icons for.
    /// @returns A vector of all icons for the specified type.
    MORE_ICONS_DLL std::vector<IconInfo> getIcons(IconType type);

    /// Returns the icon info for a specific icon.
    /// @param name The name of the icon to get the info for.
    /// @param type The type of icon to get the info for.
    /// @returns The icon info for the specified icon, or nullptr if the icon is not found.
    MORE_ICONS_DLL IconInfo* getIcon(const std::string& name, IconType type);

    /// Returns the icon info for the active icon of a specific type.
    /// @param type The type of icon to get the info for.
    /// @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
    /// @returns The icon info for the active icon of the specified type, or nullptr if the icon is not found.
    inline IconInfo* getIcon(IconType type, bool dual = false) {
        return getIcon(activeIcon(type, dual), type);
    }

    /// Returns the custom icon name of a node. (cocos2d::CCMotionStreak, CCMenuItemSpriteExtra, GJRobotSprite, PlayerObject, SimplePlayer)
    /// @param node The node to get the icon name of.
    /// @returns The icon name of the specified node, or an empty string if the icon name is not set.
    inline std::string getIconName(cocos2d::CCNode* node) {
        if (!node) return {};
        auto userObject = static_cast<cocos2d::CCString*>(node->getUserObject("name"_mi));
        return userObject ? GEODE_ANDROID(std::string)(userObject->m_sString) : std::string();
    }

    /// Creates a popup with information about an icon.
    /// @param name The name of the icon.
    /// @param type The type of the icon.
    /// @returns A pointer to the created popup, or nullptr if the popup could not be created.
    MORE_ICONS_DLL FLAlertLayer* createInfoPopup(const std::string& name, IconType type);

    #if defined(MORE_ICONS_EVENTS) || defined(GEODE_DEFINE_EVENT_EXPORTS)
    MI_INLINE cocos2d::CCTexture2D* loadIcon(const std::string& name, IconType type, int requestID)
        GEODE_EVENT_EXPORT_NORES(&loadIcon, (name, type, requestID));
    MI_INLINE void unloadIcon(const std::string& name, IconType type, int requestID)
        GEODE_EVENT_EXPORT_NORES(&unloadIcon, (name, type, requestID));
    MI_INLINE void unloadIcons(int requestID)
        GEODE_EVENT_EXPORT_NORES(&unloadIcons, (requestID));
    MI_INLINE void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type)
        GEODE_EVENT_EXPORT_NORES((void(*)(SimplePlayer*, const std::string&, IconType))(&updateSimplePlayer), (player, icon, type));
    MI_INLINE void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type)
        GEODE_EVENT_EXPORT_NORES((void(*)(GJRobotSprite*, const std::string&, IconType))(&updateRobotSprite), (sprite, icon, type));
    MI_INLINE void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type)
        GEODE_EVENT_EXPORT_NORES((void(*)(PlayerObject*, const std::string&, IconType))(&updatePlayerObject), (object, icon, type));
    MI_INLINE std::map<IconType, std::vector<IconInfo>> getIcons()
        GEODE_EVENT_EXPORT_NORES((std::map<IconType, std::vector<IconInfo>>(*)())(&getIcons), ());
    MI_INLINE std::vector<IconInfo> getIcons(IconType type)
        GEODE_EVENT_EXPORT_NORES((std::vector<IconInfo>(*)(IconType))(&more_icons::getIcons), (type));
    MI_INLINE IconInfo* getIcon(const std::string& name, IconType type)
        GEODE_EVENT_EXPORT_NORES((IconInfo*(*)(const std::string&, IconType))(&getIcon), (name, type));
    MI_INLINE FLAlertLayer* createInfoPopup(const std::string& name, IconType type)
        GEODE_EVENT_EXPORT_NORES(&createInfoPopup, (name, type));
    #endif
}
