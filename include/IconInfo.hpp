#pragma once
#include "Macros.hpp"
#include <filesystem>
#include <Geode/Enums.hpp>
#if defined(MORE_ICONS_EVENTS) || defined(GEODE_DEFINE_EVENT_EXPORTS)
#include <Geode/loader/Dispatch.hpp>
#define MI_EXPORT(fnPtr, callArgs) GEODE_EVENT_EXPORT_NORES(fnPtr, callArgs)
#else
#define MI_EXPORT(fnPtr, callArgs)
#endif
#include <matjson.hpp>

#if defined(MORE_ICONS_EVENTS)
#define MI_EXPORT_REF(fnPtr, callArgs) { \
    static auto storage = geode::geode_internal::callEventExportListener(fnPtr, GEODE_EVENT_EXPORT_ID_FOR(#fnPtr, #callArgs)); \
    if (!storage) { \
        static std::remove_const_t<std::remove_reference_t<geode::utils::function::Return<decltype(fnPtr)>>> dummy; \
        return dummy; \
    } \
    return storage callArgs; \
}
#elif defined(GEODE_DEFINE_EVENT_EXPORTS)
#define MI_EXPORT_REF(fnPtr, callArgs) GEODE_EVENT_EXPORT_NORES(fnPtr, callArgs)
#else
#define MI_EXPORT_REF(fnPtr, callArgs)
#endif

#ifdef MY_MOD_ID
#undef MY_MOD_ID
#endif
#define MY_MOD_ID "hiimjustin000.more_icons"

class IconInfoImpl;

/// A class that contains information about a custom icon.
class MORE_ICONS_DLL IconInfo {
private:
    std::shared_ptr<IconInfoImpl> m_impl;

    friend class IconInfoImpl;
    friend IconInfo* addIcon(
        std::string name, std::string shortName, IconType type, std::filesystem::path png, std::filesystem::path plist,
        std::filesystem::path json, std::filesystem::path icon, int quality, std::string packID, std::string packName,
        int specialID, matjson::Value specialInfo, int fireCount, bool vanilla, bool zipped
    );

    IconInfo(std::shared_ptr<IconInfoImpl> impl) : m_impl(std::move(impl)) {}
public:
    const std::string& getName() const MI_EXPORT_REF(&IconInfo::getName, (this));
    const std::string& getShortName() const MI_EXPORT_REF(&IconInfo::getShortName, (this));
    const std::filesystem::path& getTexture() const MI_EXPORT_REF(&IconInfo::getTexture, (this));
    std::string getTextureString() const MI_EXPORT(&IconInfo::getTextureString, (this));
    std::vector<std::string> getAllTextures() const MI_EXPORT(&IconInfo::getAllTextures, (this));
    const std::filesystem::path& getSheet() const MI_EXPORT_REF(&IconInfo::getSheet, (this));
    std::string getSheetString() const MI_EXPORT(&IconInfo::getSheetString, (this));
    const std::filesystem::path& getJSON() const MI_EXPORT_REF(&IconInfo::getJSON, (this));
    std::string getJSONString() const MI_EXPORT(&IconInfo::getJSONString, (this));
    const std::filesystem::path& getIcon() const MI_EXPORT_REF(&IconInfo::getIcon, (this));
    std::string getIconString() const MI_EXPORT(&IconInfo::getIconString, (this));
    const std::vector<std::string>& getFrameNames() const MI_EXPORT_REF(&IconInfo::getFrameNames, (this));
    const std::string& getPackName() const MI_EXPORT_REF(&IconInfo::getPackName, (this));
    const std::string& getPackID() const MI_EXPORT_REF(&IconInfo::getPackID, (this));
    IconType getType() const MI_EXPORT(&IconInfo::getType, (this));
    int getQuality() const MI_EXPORT(&IconInfo::getQuality, (this));
    int getSpecialID() const MI_EXPORT(&IconInfo::getSpecialID, (this));
    const matjson::Value& getSpecialInfo() const MI_EXPORT_REF(&IconInfo::getSpecialInfo, (this));
    int getFireCount() const MI_EXPORT(&IconInfo::getFireCount, (this));
    bool inTexturePack() const MI_EXPORT(&IconInfo::inTexturePack, (this));
    bool isVanilla() const MI_EXPORT(&IconInfo::isVanilla, (this));
    bool isZipped() const MI_EXPORT(&IconInfo::isZipped, (this));

    void setName(std::string name) MI_EXPORT(&IconInfo::setName, (this, std::move(name)));
    void setShortName(std::string shortName) MI_EXPORT(&IconInfo::setShortName, (this, std::move(shortName)));
    void setTexture(std::filesystem::path texture) MI_EXPORT(&IconInfo::setTexture, (this, std::move(texture)));
    void setSheet(std::filesystem::path sheet) MI_EXPORT(&IconInfo::setSheet, (this, std::move(sheet)));
    void setJSON(std::filesystem::path json) MI_EXPORT(&IconInfo::setJSON, (this, std::move(json)));
    void setIcon(std::filesystem::path icon) MI_EXPORT(&IconInfo::setIcon, (this, std::move(icon)));
    void setFrameNames(std::vector<std::string> frameNames) MI_EXPORT(&IconInfo::setFrameNames, (this, std::move(frameNames)));
    void setPackName(std::string packName) MI_EXPORT(&IconInfo::setPackName, (this, std::move(packName)));
    void setPackID(std::string packID) MI_EXPORT(&IconInfo::setPackID, (this, std::move(packID)));
    void setType(IconType type) MI_EXPORT(&IconInfo::setType, (this, type));
    void setQuality(int quality) MI_EXPORT(&IconInfo::setQuality, (this, quality));
    void setSpecialID(int specialID) MI_EXPORT(&IconInfo::setSpecialID, (this, specialID));
    void setSpecialInfo(matjson::Value specialInfo) MI_EXPORT(&IconInfo::setSpecialInfo, (this, std::move(specialInfo)));
    void setFireCount(int fireCount) MI_EXPORT(&IconInfo::setFireCount, (this, fireCount));
    void setVanilla(bool vanilla) MI_EXPORT(&IconInfo::setVanilla, (this, vanilla));
    void setZipped(bool zipped) MI_EXPORT(&IconInfo::setZipped, (this, zipped));

    bool equals(std::string_view name, IconType type) const
        MI_EXPORT(&IconInfo::equals, (this, name, type));
    bool equals(const IconInfo& other) const {
        return equals(other.getName(), other.getType());
    }
    bool operator==(const IconInfo& other) const {
        return equals(other);
    }

    std::strong_ordering compare(std::string_view packID, std::string_view shortName, IconType type) const
        MI_EXPORT(&IconInfo::compare, (this, packID, shortName, type));
    std::strong_ordering compare(const IconInfo& other) const {
        return compare(other.getPackID(), other.getShortName(), other.getType());
    }
    std::strong_ordering operator<=>(const IconInfo& other) const {
        return compare(other);
    }
};

#undef MI_EXPORT
#undef MI_EXPORT_REF
#undef MY_MOD_ID
