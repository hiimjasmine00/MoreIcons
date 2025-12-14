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

#ifdef MY_MOD_ID
#undef MY_MOD_ID
#endif
#define MY_MOD_ID "hiimjustin000.more_icons"

/// A class that contains information about a custom icon.
class MORE_ICONS_DLL IconInfo {
private:
    class Impl;
    std::shared_ptr<Impl> m_impl;

    friend class std::vector<IconInfo>;
    friend class IconInfoImpl;

    IconInfo(std::shared_ptr<Impl> impl) : m_impl(impl) {}
public:
    std::string getName() const MI_EXPORT(&IconInfo::getName, (this));
    std::string getShortName() const MI_EXPORT(&IconInfo::getShortName, (this));
    std::filesystem::path getTexture() const MI_EXPORT(&IconInfo::getTexture, (this));
    std::string getTextureString() const MI_EXPORT(&IconInfo::getTextureString, (this));
    std::vector<std::string> getAllTextures() const MI_EXPORT(&IconInfo::getAllTextures, (this));
    std::filesystem::path getSheet() const MI_EXPORT(&IconInfo::getSheet, (this));
    std::string getSheetString() const MI_EXPORT(&IconInfo::getSheetString, (this));
    std::filesystem::path getJSON() const MI_EXPORT(&IconInfo::getJSON, (this));
    std::string getJSONString() const MI_EXPORT(&IconInfo::getJSONString, (this));
    std::filesystem::path getIcon() const MI_EXPORT(&IconInfo::getIcon, (this));
    std::string getIconString() const MI_EXPORT(&IconInfo::getIconString, (this));
    std::vector<std::string> getFrameNames() const MI_EXPORT(&IconInfo::getFrameNames, (this));
    std::string getPackName() const MI_EXPORT(&IconInfo::getPackName, (this));
    std::string getPackID() const MI_EXPORT(&IconInfo::getPackID, (this));
    IconType getType() const MI_EXPORT(&IconInfo::getType, (this));
    int getQuality() const MI_EXPORT(&IconInfo::getQuality, (this));
    int getSpecialID() const MI_EXPORT(&IconInfo::getSpecialID, (this));
    matjson::Value getSpecialInfo() const MI_EXPORT(&IconInfo::getSpecialInfo, (this));
    int getFireCount() const MI_EXPORT(&IconInfo::getFireCount, (this));
    bool inTexturePack() const MI_EXPORT(&IconInfo::inTexturePack, (this));
    bool isVanilla() const MI_EXPORT(&IconInfo::isVanilla, (this));
    bool isZipped() const MI_EXPORT(&IconInfo::isZipped, (this));

    void setName(const std::string& name) MI_EXPORT(&IconInfo::setName, (this, name));
    void moveName(std::string&& name) MI_EXPORT(&IconInfo::moveName, (this, std::move(name)));

    void setShortName(const std::string& shortName) MI_EXPORT(&IconInfo::setShortName, (this, shortName));
    void moveShortName(std::string&& shortName) MI_EXPORT(&IconInfo::moveShortName, (this, std::move(shortName)));

    void setTexture(const std::filesystem::path& texture) MI_EXPORT(&IconInfo::setTexture, (this, texture));
    void moveTexture(std::filesystem::path&& texture) MI_EXPORT(&IconInfo::moveTexture, (this, std::move(texture)));

    void setSheet(const std::filesystem::path& sheet) MI_EXPORT(&IconInfo::setSheet, (this, sheet));
    void moveSheet(std::filesystem::path&& sheet) MI_EXPORT(&IconInfo::moveSheet, (this, std::move(sheet)));

    void setJSON(const std::filesystem::path& json) MI_EXPORT(&IconInfo::setJSON, (this, json));
    void moveJSON(std::filesystem::path&& json) MI_EXPORT(&IconInfo::moveJSON, (this, std::move(json)));

    void setIcon(const std::filesystem::path& icon) MI_EXPORT(&IconInfo::setIcon, (this, icon));
    void moveIcon(std::filesystem::path&& icon) MI_EXPORT(&IconInfo::moveIcon, (this, std::move(icon)));

    void setFrameNames(const std::vector<std::string>& frameNames) MI_EXPORT(&IconInfo::setFrameNames, (this, frameNames));
    void moveFrameNames(std::vector<std::string>&& frameNames) MI_EXPORT(&IconInfo::moveFrameNames, (this, std::move(frameNames)));

    void setPackName(const std::string& packName) MI_EXPORT(&IconInfo::setPackName, (this, packName));
    void movePackName(std::string&& packName) MI_EXPORT(&IconInfo::movePackName, (this, std::move(packName)));

    void setPackID(const std::string& packID) MI_EXPORT(&IconInfo::setPackID, (this, packID));
    void movePackID(std::string&& packID) MI_EXPORT(&IconInfo::movePackID, (this, std::move(packID)));

    void setType(IconType type) MI_EXPORT(&IconInfo::setType, (this, type));
    void setQuality(int quality) MI_EXPORT(&IconInfo::setQuality, (this, quality));
    void setSpecialID(int specialID) MI_EXPORT(&IconInfo::setSpecialID, (this, specialID));

    void setSpecialInfo(const matjson::Value& specialInfo) MI_EXPORT(&IconInfo::setSpecialInfo, (this, specialInfo));
    void moveSpecialInfo(matjson::Value&& specialInfo) MI_EXPORT(&IconInfo::moveSpecialInfo, (this, std::move(specialInfo)));

    void setFireCount(int fireCount) MI_EXPORT(&IconInfo::setFireCount, (this, fireCount));
    void setVanilla(bool vanilla) MI_EXPORT(&IconInfo::setVanilla, (this, vanilla));
    void setZipped(bool zipped) MI_EXPORT(&IconInfo::setZipped, (this, zipped));

    bool operator==(const IconInfo& other) const {
        return equals(other);
    }
    bool equals(const IconInfo& other) const;
    bool equals(const std::string& name, IconType type) const;
    std::strong_ordering operator<=>(const IconInfo& other) const {
        return compare(other) <=> 0;
    }
    int compare(const IconInfo& other) const;
    int compare(const std::string& packID2, const std::string& shortName2, IconType type2) const;
};

#undef MI_EXPORT
#undef MY_MOD_ID
