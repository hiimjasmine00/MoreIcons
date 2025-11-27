#pragma once
#include "TrailInfo.hpp"
#include <Geode/Enums.hpp>

/// A struct that contains information about a custom icon.
struct IconInfo {
    std::string name;
    std::vector<std::string> textures;
    std::vector<std::string> frameNames;
    std::string sheetName;
    std::string packName;
    std::string packID;
    IconType type;
    int trailID;
    TrailInfo trailInfo;
    std::string shortName;
    bool vanilla;
    bool zipped;

    IconInfo(
        const std::string& name, const std::string& texture, const std::string& sheetName,
        const std::string& packName, const std::string& packID, IconType type, int trailID,
        const TrailInfo& trailInfo, const std::string& shortName, bool vanilla, bool zipped
    ) : name(name), textures({ texture }), sheetName(sheetName), packName(packName),
        packID(packID), type(type), trailID(trailID), trailInfo(trailInfo),
        shortName(shortName), vanilla(vanilla), zipped(zipped) {}

    bool operator==(const IconInfo& other) const {
        return type == other.type && name == other.name;
    }

    std::strong_ordering operator<=>(const IconInfo& other) const {
        return compare(other) <=> 0;
    }

    int compare(const IconInfo& other) const {
        return compare(other.packID, other.shortName, other.type);
    }

    int compare(const std::string& packID2, const std::string& shortName2, IconType type2) const {
        auto comparison = type <=> type2;
        if (comparison != 0) return comparison < 0 ? -1 : 1;

        auto samePack = packID == packID2;
        if (samePack && shortName == shortName2) return 0;
        if (packID.empty() && !packID2.empty()) return -1;
        if (!packID.empty() && packID2.empty()) return 1;

        std::string_view a = samePack ? shortName : packID;
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
};
