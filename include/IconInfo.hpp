#pragma once
#include <Geode/Enums.hpp>
#include <string>
#include <vector>

/**
 * A struct that contains information about a custom icon.
 */
struct IconInfo {
    std::string name;
    std::vector<std::string> textures;
    std::vector<std::string> frameNames;
    std::string sheetName;
    std::string packName;
    std::string packID;
    IconType type;
    int trailID;
    bool blend;
    bool tint;
    bool show;
    float fade;
    float stroke;
    std::string shortName;
    bool vanilla;
    bool zipped;

    bool operator==(const IconInfo& other) const {
        return name == other.name && type == other.type;
    }

    std::strong_ordering operator<=>(const IconInfo& other) const {
        if (type != other.type) return type <=> other.type;
        if (name == other.name) return std::strong_ordering::equal;
        if (packID.empty() && !other.packID.empty()) return std::strong_ordering::less;
        if (!packID.empty() && other.packID.empty()) return std::strong_ordering::greater;

        auto differentPack = packID != other.packID;
        std::string_view a = differentPack ? packID : shortName, b = differentPack ? other.packID : other.shortName;

        for (size_t aIt = 0, bIt = 0; aIt < a.size() && bIt < b.size();) {
            if (isdigit(a[aIt]) && isdigit(b[bIt])) {
                auto aStart = aIt, bStart = bIt;
                for (; aIt < a.size() && isdigit(a[aIt]); aIt++);
                for (; bIt < b.size() && isdigit(b[bIt]); bIt++);
                auto aNum = a.substr(aStart, aIt - aStart), bNum = b.substr(bStart, bIt - bStart);
                auto comparison = aNum.size() != bNum.size() ? aNum.size() <=> bNum.size() : aNum <=> bNum;
                if (comparison != std::strong_ordering::equal) return comparison;
            }
            else {
                auto comparison = tolower(a[aIt++]) <=> tolower(b[bIt++]);
                if (comparison != std::strong_ordering::equal) return comparison;
            }
        }

        return a.size() <=> b.size();
    }
};
