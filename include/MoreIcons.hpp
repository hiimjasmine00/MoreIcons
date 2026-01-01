#pragma once
#include "IconInfo.hpp"
#include <Geode/binding/GJRobotSprite.hpp>
#include <Geode/binding/PlayerObject.hpp>
#if defined(MORE_ICONS_EVENTS) || defined(GEODE_DEFINE_EVENT_EXPORTS)
#include <Geode/loader/Dispatch.hpp>
#define MI_EXP(fnPtr, callArgs) GEODE_EVENT_EXPORT_NORES(fnPtr, callArgs)
#else
#define MI_EXP(fnPtr, callArgs)
#endif
#include <Geode/loader/Mod.hpp>

#if defined(MORE_ICONS_EVENTS) && !defined(GEODE_DEFINE_EVENT_EXPORTS)
#define MI_EXP_REF(fnPtr, callArgs) { \
    static auto storage = geode::geode_internal::callEventExportListener(fnPtr, GEODE_EVENT_EXPORT_ID_FOR(#fnPtr, #callArgs)); \
    if (!storage) { \
        static std::remove_const_t<std::remove_reference_t<geode::utils::function::Return<decltype(fnPtr)>>> dummy; \
        return dummy; \
    } \
    return storage callArgs; \
}
#elif defined(GEODE_DEFINE_EVENT_EXPORTS)
#define MI_EXP_REF(fnPtr, callArgs) GEODE_EVENT_EXPORT_NORES(fnPtr, callArgs)
#else
#define MI_EXP_REF(fnPtr, callArgs)
#endif

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
        if (dual && geode::Loader::get()->isModLoaded("weebify.separate_dual_icons")) {
            switch (type) {
                case IconType::Cube: return "icon-dual";
                case IconType::Ship: return "ship-dual";
                case IconType::Ball: return "ball-dual";
                case IconType::Ufo: return "ufo-dual";
                case IconType::Wave: return "wave-dual";
                case IconType::Robot: return "robot-dual";
                case IconType::Spider: return "spider-dual";
                case IconType::Swing: return "swing-dual";
                case IconType::Jetpack: return "jetpack-dual";
                case IconType::DeathEffect: return "death-dual";
                case IconType::Special: return "trail-dual";
                case IconType::ShipFire: return "fire-dual";
                default: return "";
            }
        }
        else {
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
    inline std::string setIcon(std::string icon, IconType type, bool dual = false) {
        auto mod = get();
        if (!mod) return {};
        auto savedType = saveKey(type, dual);
        if (savedType.empty()) return {};
        auto& saveValue = mod->getSaveContainer()[savedType];
        auto previous = saveValue.asString().unwrapOrDefault();
        saveValue = std::move(icon);
        return previous;
    }

    /// Loads a custom icon into the texture cache.
    /// @param name The name of the icon to load.
    /// @param type The type of icon to load.
    /// @param requestID The request ID of the icon to load.
    MI_DLL cocos2d::CCTexture2D* loadIcon(std::string_view name, IconType type, int requestID) MI_EXP(&loadIcon, (name, type, requestID));

    /// Unloads a custom icon from the texture cache.
    /// @param name The name of the icon to unload.
    /// @param type The type of icon to unload.
    /// @param requestID The request ID of the icon to unload.
    MI_DLL void unloadIcon(std::string_view name, IconType type, int requestID) MI_EXP(&unloadIcon, (name, type, requestID));

    /// Unloads all custom icons associated with a request ID.
    /// @param requestID The request ID of the icons to unload.
    MI_DLL void unloadIcons(int requestID) MI_EXP(&unloadIcons, (requestID));

    /// Changes the icon of a SimplePlayer object to a custom icon.
    /// @param player The SimplePlayer object to change the icon of.
    /// @param icon The name of the icon to change to.
    /// @param type The type of icon to change to.
    MI_DLL void updateSimplePlayer(SimplePlayer* player, std::string_view icon, IconType type) MI_EXP(&updateSimplePlayer, (player, icon, type));

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
    MI_DLL void updateRobotSprite(GJRobotSprite* sprite, std::string_view icon, IconType type) MI_EXP(&updateRobotSprite, (sprite, icon, type));

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
    inline void updateRobotSprite(GJRobotSprite* sprite, std::string_view icon) {
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
    MI_DLL void updatePlayerObject(PlayerObject* object, std::string_view icon, IconType type) MI_EXP(&updatePlayerObject, (object, icon, type));

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
    inline void updatePlayerObject(PlayerObject* object, std::string_view icon) {
        if (object && !icon.empty()) updatePlayerObject(object, icon, getIconType(object));
    }

    /// Returns a map of all custom icons.
    /// @returns A map of all custom icons.
    MI_DLL const std::map<IconType, std::vector<IconInfo>>& getAllIcons() MI_EXP_REF(&getAllIcons, ());

    /// Returns a pointer to a vector of all icons for a specific type.
    /// @param type The type of icon to get all icons for.
    /// @returns A pointer to a vector of all icons for the specified type, or nullptr if the mod is not loaded or the type is unsupported.
    MI_DLL std::vector<IconInfo>* getIcons(IconType type) MI_EXP(&getIcons, (type));

    /// Returns the icon info for a specific icon.
    /// @param name The name of the icon to get the info for.
    /// @param type The type of icon to get the info for.
    /// @returns The icon info for the specified icon, or nullptr if the icon is not found.
    MI_DLL IconInfo* getIcon(std::string_view name, IconType type) MI_EXP(&getIcon, (name, type));

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
    inline bool hasIcon(std::string_view name, IconType type) {
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
    MI_DLL FLAlertLayer* createInfoPopup(std::string_view name, IconType type) MI_EXP(&createInfoPopup, (name, type));

    /// Adds an icon to the icon list.
    /// @param name The name of the icon.
    /// @param shortName The name of the icon without the pack prefix.
    /// @param type The type of the icon.
    /// @param png The path to the icon's PNG file.
    /// @param plist The path to the icon's Plist file.
    /// @param quality The texture quality of the icon.
    /// @param packID The ID of the icon pack.
    /// @param packName The name of the icon pack.
    /// @param vanilla Whether or not the icon is a vanilla icon.
    /// @param zipped Whether or not the icon is in a zipped pack.
    /// @returns A pointer to the added icon info, or nullptr if the mod is not loaded or the type is unsupported.
    MI_DLL IconInfo* addIcon(
        std::string name, std::string shortName, IconType type, std::filesystem::path png, std::filesystem::path plist,
        cocos2d::TextureQuality quality, std::string packID = {}, std::string packName = std::string("More Icons", 10),
        bool vanilla = false, bool zipped = false
    ) MI_EXP(&addIcon, (
        std::move(name), std::move(shortName), type, std::move(png), std::move(plist),
        quality, std::move(packID), std::move(packName), vanilla, zipped
    ));

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
        std::string name, std::string shortName, std::filesystem::path png, std::filesystem::path json, std::filesystem::path icon,
        std::string packID = {}, std::string packName = std::string("More Icons", 10), int specialID = 0, matjson::Value specialInfo = {},
        bool vanilla = false, bool zipped = false
    ) MI_EXP(&addTrail, (
        std::move(name), std::move(shortName), std::move(png), std::move(json), std::move(icon),
        std::move(packID), std::move(packName), specialID, std::move(specialInfo), vanilla, zipped
    ));

    /// Adds a death effect to the icon list.
    /// @param name The name of the death effect.
    /// @param shortName The name of the death effect without the pack prefix.
    /// @param png The path to the death effect's PNG file.
    /// @param plist The path to the death effect's Plist file.
    /// @param json The path to the death effect's JSON file.
    /// @param icon The path to the death effect's icon file.
    /// @param quality The texture quality of the death effect.
    /// @param packID The ID of the icon pack.
    /// @param packName The name of the icon pack.
    /// @param specialID The ID of the vanilla death effect this death effect originates from.
    /// @param specialInfo The special info of the death effect.
    /// @param vanilla Whether or not the death effect is a vanilla death effect.
    /// @param zipped Whether or not the death effect is in a zipped pack.
    /// @returns A pointer to the added icon info, or nullptr if the mod is not loaded.
    MI_DLL IconInfo* addDeathEffect(
        std::string name, std::string shortName, std::filesystem::path png, std::filesystem::path plist,
        std::filesystem::path json, std::filesystem::path icon, cocos2d::TextureQuality quality,
        std::string packID = {}, std::string packName = std::string("More Icons", 10), int specialID = 0, matjson::Value specialInfo = {},
        bool vanilla = false, bool zipped = false
    ) MI_EXP(&addDeathEffect, (
        std::move(name), std::move(shortName), std::move(png), std::move(plist), std::move(json), std::move(icon),
        quality, std::move(packID), std::move(packName), specialID, std::move(specialInfo), vanilla, zipped
    ));

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
        std::string name, std::string shortName,
        std::filesystem::path png, std::filesystem::path json, std::filesystem::path icon,
        std::string packID = {}, std::string packName = std::string("More Icons", 10),
        int specialID = 0, matjson::Value specialInfo = {},
        int fireCount = 0, bool vanilla = false, bool zipped = false
    ) MI_EXP(&addShipFire, (
        std::move(name), std::move(shortName), std::move(png), std::move(json), std::move(icon),
        std::move(packID), std::move(packName), specialID, std::move(specialInfo), fireCount, vanilla, zipped
    ));

    /// Moves an icon to a different directory.
    /// @param info The icon info of the icon to move.
    /// @param path The path of the directory to move the icon to.
    MI_DLL void moveIcon(IconInfo* info, const std::filesystem::path& path) MI_EXP(&moveIcon, (info, path));

    /// Removes an icon from the icon list.
    /// @param info The icon info of the icon to remove.
    MI_DLL void removeIcon(IconInfo* info) MI_EXP(&removeIcon, (info));

    /// Renames an icon.
    /// @param info The icon info of the icon to rename.
    /// @param name The new name of the icon.
    MI_DLL void renameIcon(IconInfo* info, std::string name) MI_EXP(&renameIcon, (info, std::move(name)));

    /// Updates an icon's image and sheet data.
    /// @param info The icon info of the icon to update.
    MI_DLL void updateIcon(IconInfo* info) MI_EXP(&updateIcon, (info));
}

#undef MI_EXP
#undef MI_EXP_REF
#undef MY_MOD_ID
