#include "IconInfoImpl.hpp"
#include <fmt/format.h>
#include <Geode/utils/string.hpp>

using namespace geode::prelude;

const std::string& IconInfo::getName() const {
    return m_impl->m_name;
}

const std::string& IconInfo::getShortName() const {
    return m_impl->m_shortName;
}

const std::filesystem::path& IconInfo::getTexture() const {
    return m_impl->m_texture;
}

std::string IconInfo::getTextureString() const {
    return string::pathToString(m_impl->m_texture);
}

std::vector<std::string> IconInfo::getAllTextures() const {
    std::vector<std::string> textures;
    if (m_impl->m_type == IconType::ShipFire) {
        auto texture = getTextureString();
        for (int i = 1; i <= m_impl->m_fireCount; i++) {
            texture.replace(texture.size() - 7, 3, fmt::format("{:03}", i));
            textures.push_back(texture);
        }
    }
    else textures.push_back(getTextureString());
    return textures;
}

const std::filesystem::path& IconInfo::getSheet() const {
    return m_impl->m_sheet;
}

std::string IconInfo::getSheetString() const {
    return string::pathToString(m_impl->m_sheet);
}

const std::filesystem::path& IconInfo::getJSON() const {
    return m_impl->m_json;
}

std::string IconInfo::getJSONString() const {
    return string::pathToString(m_impl->m_json);
}

const std::filesystem::path& IconInfo::getIcon() const {
    return m_impl->m_icon;
}

std::string IconInfo::getIconString() const {
    return string::pathToString(m_impl->m_icon);
}

const std::vector<std::string>& IconInfo::getFrameNames() const {
    return m_impl->m_frameNames;
}

const std::string& IconInfo::getPackName() const {
    return m_impl->m_packName;
}

const std::string& IconInfo::getPackID() const {
    return m_impl->m_packID;
}

IconType IconInfo::getType() const {
    return m_impl->m_type;
}

int IconInfo::getQuality() const {
    return m_impl->m_quality;
}

int IconInfo::getSpecialID() const {
    return m_impl->m_specialID;
}

const matjson::Value& IconInfo::getSpecialInfo() const {
    return m_impl->m_specialInfo;
}

int IconInfo::getFireCount() const {
    return m_impl->m_fireCount;
}

bool IconInfo::inTexturePack() const {
    return !m_impl->m_packID.empty();
}

bool IconInfo::isVanilla() const {
    return m_impl->m_vanilla;
}

bool IconInfo::isZipped() const {
    return m_impl->m_zipped;
}

void IconInfo::setName(std::string name) {
    m_impl->m_name = std::move(name);
}

void IconInfo::setShortName(std::string shortName) {
    m_impl->m_shortName = std::move(shortName);
}

void IconInfo::setTexture(std::filesystem::path texture) {
    m_impl->m_texture = std::move(texture);
}

void IconInfo::setSheet(std::filesystem::path sheet) {
    m_impl->m_sheet = std::move(sheet);
}

void IconInfo::setJSON(std::filesystem::path json) {
    m_impl->m_json = std::move(json);
}

    void IconInfo::setIcon(std::filesystem::path icon) {
    m_impl->m_icon = std::move(icon);
}

void IconInfo::setFrameNames(std::vector<std::string> frameNames) {
    m_impl->m_frameNames = std::move(frameNames);
}

void IconInfo::setPackName(std::string packName) {
    m_impl->m_packName = std::move(packName);
}

void IconInfo::setPackID(std::string packID) {
    m_impl->m_packID = std::move(packID);
}

void IconInfo::setType(IconType type) {
    m_impl->m_type = type;
}

void IconInfo::setQuality(int quality) {
    m_impl->m_quality = quality;
}

void IconInfo::setSpecialID(int specialID) {
    m_impl->m_specialID = specialID;
}

void IconInfo::setSpecialInfo(matjson::Value specialInfo) {
    m_impl->m_specialInfo = std::move(specialInfo);
}

void IconInfo::setFireCount(int fireCount) {
    m_impl->m_fireCount = fireCount;
}

void IconInfo::setVanilla(bool vanilla) {
    m_impl->m_vanilla = vanilla;
}

void IconInfo::setZipped(bool zipped) {
    m_impl->m_zipped = zipped;
}

bool IconInfo::equals(const IconInfo& other) const {
    return equals(other.m_impl->m_name, other.m_impl->m_type);
}

bool IconInfo::equals(const std::string& name, IconType type) const {
    return m_impl->m_name == name && m_impl->m_type == type;
}

int IconInfo::compare(const IconInfo& other) const {
    return compare(other.m_impl->m_packID, other.m_impl->m_shortName, other.m_impl->m_type);
}

int IconInfo::compare(const std::string& packID2, const std::string& shortName2, IconType type2) const {
    auto comparison = m_impl->m_type <=> type2;
    if (comparison != 0) return comparison < 0 ? -1 : 1;

    auto& packID1 = m_impl->m_packID;
    auto& shortName1 = m_impl->m_shortName;
    auto samePack = packID1 == packID2;
    if (samePack && shortName1 == shortName2) return 0;
    if (packID1.empty() && !packID2.empty()) return -1;
    if (!packID1.empty() && packID2.empty()) return 1;

    std::string_view a = samePack ? shortName1 : packID1;
    std::string_view b = samePack ? shortName2 : packID2;

    for (size_t aIt = 0, bIt = 0; aIt < a.size() && bIt < b.size();) {
        if (isdigit(a[aIt]) && isdigit(b[bIt])) {
            auto aStart = aIt;
            while (aIt < a.size() && isdigit(a[aIt])) aIt++;
            auto aSize = aIt - aStart;

            auto bStart = bIt;
            while (bIt < b.size() && isdigit(b[bIt])) bIt++;
            auto bSize = bIt - bStart;

            comparison = aSize == bSize ? a.substr(aStart, aSize) <=> b.substr(bStart, bSize) : aSize <=> bSize;
        }
        else comparison = tolower(a[aIt++]) <=> tolower(b[bIt++]);

        if (comparison != 0) return comparison < 0 ? -1 : 1;
    }

    return a.size() < b.size() ? -1 : a.size() > b.size() ? 1 : 0;
}
