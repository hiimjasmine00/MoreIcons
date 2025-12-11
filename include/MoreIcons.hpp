#pragma once
#include "IconInfo.hpp"
#include <Geode/binding/GJRobotSprite.hpp>
#include <Geode/binding/PlayerObject.hpp>
#if defined(MORE_ICONS_EVENTS) || defined(GEODE_DEFINE_EVENT_EXPORTS)
#include <Geode/loader/Dispatch.hpp>
#define MI_EXPORT(fnPtr, callArgs) GEODE_EVENT_EXPORT_NORES(fnPtr, callArgs)
#else
#define MI_EXPORT(fnPtr, callArgs)
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
                for (size_t i = 0; i < sizeof(id); i++) {
                    buffer[i] = id[i];
                }
                buffer[extra - 1] = '/';
                for (size_t i = 0; i < N; i++) {
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

#ifdef MORE_ICONS_EVENTS
#define MI_DLL inline
#else
#define MI_DLL MORE_ICONS_DLL
#endif

namespace more_icons {
    /// Checks if the More Icons mod is loaded.
    /// @returns Whether or not the More Icons mod is loaded.
    inline bool loaded() {
        return geode::Loader::get()->isModLoaded(std::string(ID));
    }

    /// Returns the Mod object for the More Icons mod.
    /// @returns The Mod object for the More Icons mod, or nullptr if the mod is not loaded.
    inline geode::Mod* get() {
        return geode::Loader::get()->getLoadedMod(std::string(ID));
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
    MI_DLL cocos2d::CCTexture2D* loadIcon(const std::string& name, IconType type, int requestID)
        MI_EXPORT(&loadIcon, (name, type, requestID));

    /// Unloads a custom icon from the texture cache.
    /// @param name The name of the icon to unload.
    /// @param type The type of icon to unload.
    /// @param requestID The request ID of the icon to unload.
    MI_DLL void unloadIcon(const std::string& name, IconType type, int requestID)
        MI_EXPORT(&unloadIcon, (name, type, requestID));

    /// Unloads all custom icons associated with a request ID.
    /// @param requestID The request ID of the icons to unload.
    MI_DLL void unloadIcons(int requestID)
        MI_EXPORT(&unloadIcons, (requestID));

    /// Changes the icon of a SimplePlayer object to a custom icon.
    /// @param player The SimplePlayer object to change the icon of.
    /// @param icon The name of the icon to change to.
    /// @param type The type of icon to change to.
    MI_DLL void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type)
        MI_EXPORT((void(*)(SimplePlayer*, const std::string&, IconType))(&updateSimplePlayer), (player, icon, type));

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
    MI_DLL void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type)
        MI_EXPORT((void(*)(GJRobotSprite*, const std::string&, IconType))(&updateRobotSprite), (sprite, icon, type));

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
    MI_DLL void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type)
        MI_EXPORT((void(*)(PlayerObject*, const std::string&, IconType))(&updatePlayerObject), (object, icon, type));

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

    /// Returns a pointer to the map of all custom icons.
    /// @returns A pointer to the map of all custom icons, or nullptr if the mod is not loaded.
    MI_DLL std::map<IconType, std::vector<IconInfo>>* getIcons()
        MI_EXPORT((std::map<IconType, std::vector<IconInfo>>*(*)())(&getIcons), ());

    /// Returns a pointer to a vector of all icons for a specific type.
    /// @param type The type of icon to get all icons for.
    /// @returns A pointer to a vector of all icons for the specified type, or nullptr if the mod is not loaded or the type is unsupported.
    MI_DLL std::vector<IconInfo>* getIcons(IconType type)
        MI_EXPORT((std::vector<IconInfo>*(*)(IconType))(&getIcons), (type));

    /// Returns the icon info for a specific icon.
    /// @param name The name of the icon to get the info for.
    /// @param type The type of icon to get the info for.
    /// @returns The icon info for the specified icon, or nullptr if the icon is not found.
    MI_DLL IconInfo* getIcon(const std::string& name, IconType type)
        MI_EXPORT((IconInfo*(*)(const std::string&, IconType))(&getIcon), (name, type));

    /// Returns the number of icons for a specific type.
    /// @param type The type of icon to get the count for.
    /// @returns The number of icons for the specified type.
    inline size_t getIconCount(IconType type) {
        auto icons = getIcons(type);
        return icons ? icons->size() : 0;
    }

    /// Returns the icon info for the active icon of a specific type.
    /// @param type The type of icon to get the info for.
    /// @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
    /// @returns The icon info for the active icon of the specified type, or nullptr if the icon is not found.
    inline IconInfo* getIcon(IconType type, bool dual = false) {
        return getIcon(activeIcon(type, dual), type);
    }

    /// Checks if a specific icon exists.
    /// @param name The name of the icon to check for.
    /// @param type The type of icon to check for.
    /// @returns Whether or not the specified icon exists.
    inline bool hasIcon(const std::string& name, IconType type) {
        return getIcon(name, type) != nullptr;
    }

    /// Checks if the active icon of a specific type exists.
    /// @param type The type of icon to check for.
    /// @param dual Whether or not to use the icon for the dual player. (Requires the "Separate Dual Icons" mod by Weebify)
    /// @returns Whether or not the active icon of the specified type exists.
    inline bool hasIcon(IconType type, bool dual = false) {
        return hasIcon(activeIcon(type, dual), type);
    }

    /// Returns the custom icon name of a node. (cocos2d::CCMotionStreak, CCMenuItemSpriteExtra, GJRobotSprite, PlayerObject, SimplePlayer)
    /// @param node The node to get the icon name of.
    /// @returns The icon name of the specified node, or an empty string if the icon name is not set.
    inline std::string getIconName(cocos2d::CCNode* node) {
        if (!node) return {};
        auto userObject = static_cast<cocos2d::CCString*>(node->getUserObject("name"_mi));
        return userObject ? std::string(userObject->m_sString) : std::string();
    }

    /// Creates a popup with information about an icon.
    /// @param name The name of the icon.
    /// @param type The type of the icon.
    /// @returns A pointer to the created popup, or nullptr if the popup could not be created.
    MI_DLL FLAlertLayer* createInfoPopup(const std::string& name, IconType type)
        MI_EXPORT(&createInfoPopup, (name, type));

    /// Adds an icon to the icon list.
    /// @param name The name of the icon.
    /// @param shortName The name of the icon without the pack prefix.
    /// @param type The type of the icon.
    /// @param png The path to the icon's PNG file.
    /// @param plist The path to the icon's Plist file.
    /// @param packID The ID of the icon pack.
    /// @param packName The name of the icon pack.
    /// @param vanilla Whether or not the icon is a vanilla icon.
    /// @param zipped Whether or not the icon is in a zipped pack.
    /// @returns A pointer to the added icon info, or nullptr if the mod is not loaded or the type is unsupported.
    MI_DLL IconInfo* addIcon(
        const std::string& name, const std::string& shortName, IconType type,
        const std::filesystem::path& png, const std::filesystem::path& plist,
        const std::string& packID = {}, const std::string& packName = "More Icons",
        bool vanilla = false, bool zipped = false
    )
        MI_EXPORT(&addIcon, (name, shortName, type, png, plist, packID, packName, vanilla, zipped));

    /// Adds a trail to the icon list.
    /// @param name The name of the trail.
    /// @param shortName The name of the trail without the pack prefix.
    /// @param png The path to the trail's PNG file.
    /// @param json The path to the trail's JSON file.
    /// @param icon The path to the trail's icon file.
    /// @param packID The ID of the icon pack.
    /// @param packName The name of the icon pack.
    /// @param specialID The ID of the vanilla trail this trail originates from.
    /// @param specialInfo The special info of the trail.
    /// @param vanilla Whether or not the trail is a vanilla trail.
    /// @param zipped Whether or not the trail is in a zipped pack.
    /// @returns A pointer to the added icon info, or nullptr if the mod is not loaded.
    MI_DLL IconInfo* addTrail(
        const std::string& name, const std::string& shortName,
        const std::filesystem::path& png, const std::filesystem::path& json, const std::filesystem::path& icon,
        const std::string& packID = {}, const std::string& packName = "More Icons",
        int specialID = 0, const matjson::Value& specialInfo = {},
        bool vanilla = false, bool zipped = false
    )
        MI_EXPORT(&addTrail, (name, shortName, png, json, icon, packID, packName, specialID, specialInfo, vanilla, zipped));

    /// Adds a death effect to the icon list.
    /// @param name The name of the death effect.
    /// @param shortName The name of the death effect without the pack prefix.
    /// @param png The path to the death effect's PNG file.
    /// @param plist The path to the death effect's Plist file.
    /// @param json The path to the death effect's JSON file.
    /// @param icon The path to the death effect's icon file.
    /// @param packID The ID of the icon pack.
    /// @param packName The name of the icon pack.
    /// @param specialID The ID of the vanilla death effect this death effect originates from.
    /// @param specialInfo The special info of the death effect.
    /// @param vanilla Whether or not the death effect is a vanilla death effect.
    /// @param zipped Whether or not the death effect is in a zipped pack.
    /// @returns A pointer to the added icon info, or nullptr if the mod is not loaded.
    MI_DLL IconInfo* addDeathEffect(
        const std::string& name, const std::string& shortName,
        const std::filesystem::path& png, const std::filesystem::path& plist,
        const std::filesystem::path& json, const std::filesystem::path& icon,
        const std::string& packID = {}, const std::string& packName = "More Icons",
        int specialID = 0, const matjson::Value& specialInfo = {},
        bool vanilla = false, bool zipped = false
    )
        MI_EXPORT(&addDeathEffect, (name, shortName, png, plist, json, icon, packID, packName, specialID, specialInfo, vanilla, zipped));

    /// Adds a ship fire to the icon list.
    /// @param name The name of the ship fire.
    /// @param shortName The name of the ship fire without the pack prefix.
    /// @param png The path to the ship fire's PNG file.
    /// @param json The path to the ship fire's JSON file.
    /// @param icon The path to the ship fire's icon file.
    /// @param packID The ID of the icon pack.
    /// @param packName The name of the icon pack.
    /// @param specialID The ID of the vanilla ship fire this ship fire originates from.
    /// @param specialInfo The special info of the ship fire.
    /// @param fireCount The amount of frames in the fire animation.
    /// @param vanilla Whether or not the ship fire is a vanilla ship fire.
    /// @param zipped Whether or not the ship fire is in a zipped pack.
    /// @returns A pointer to the added icon info, or nullptr if the mod is not loaded.
    MI_DLL IconInfo* addShipFire(
        const std::string& name, const std::string& shortName,
        const std::filesystem::path& png, const std::filesystem::path& json, const std::filesystem::path& icon,
        const std::string& packID = {}, const std::string& packName = "More Icons",
        int specialID = 0, const matjson::Value& specialInfo = {},
        int fireCount = 0, bool vanilla = false, bool zipped = false
    )
        MI_EXPORT(&addShipFire, (name, shortName, png, json, icon, packID, packName, specialID, specialInfo, fireCount, vanilla, zipped));

    /// Moves an icon to a different directory.
    /// @param info The icon info of the icon to move.
    /// @param path The path of the directory to move the icon to.
    MI_DLL void moveIcon(IconInfo* info, const std::filesystem::path& path)
        MI_EXPORT(&moveIcon, (info, path));

    /// Removes an icon from the icon list.
    /// @param info The icon info of the icon to remove.
    MI_DLL void removeIcon(IconInfo* info)
        MI_EXPORT(&removeIcon, (info));

    /// Renames an icon.
    /// @param info The icon info of the icon to rename.
    /// @param name The new name of the icon.
    MI_DLL void renameIcon(IconInfo* info, const std::string& name)
        MI_EXPORT(&renameIcon, (info, name));

    /// Updates an icon's image and sheet data.
    /// @param info The icon info of the icon to update.
    MI_DLL void updateIcon(IconInfo* info)
        MI_EXPORT(&updateIcon, (info));
}

#undef MI_EXPORT
#undef MY_MOD_ID
