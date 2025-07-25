#pragma once
#include <Geode/Enums.hpp>
#include <matjson.hpp>

/// A struct that contains information about a custom trail.
struct TrailInfo {
    bool blend = false;
    bool tint = false;
    bool show = false;
    float fade = 0.3f;
    float stroke = 14.0f;

    bool operator==(const TrailInfo& other) const {
        return blend == other.blend && tint == other.tint && show == other.show && fade == other.fade && stroke == other.stroke;
    }
};

template <>
struct matjson::Serialize<TrailInfo> {
    static geode::Result<TrailInfo> fromJson(const matjson::Value& value) {
        TrailInfo info;
        if (value.isObject()) {
            GEODE_UNWRAP_INTO_IF_OK(info.blend, value.get("blend").andThen([](const matjson::Value& v) { return v.asBool(); }));
            GEODE_UNWRAP_INTO_IF_OK(info.tint, value.get("tint").andThen([](const matjson::Value& v) { return v.asBool(); }));
            GEODE_UNWRAP_INTO_IF_OK(info.show, value.get("show").andThen([](const matjson::Value& v) { return v.asBool(); }));
            GEODE_UNWRAP_INTO_IF_OK(info.fade, value.get("fade").andThen([](const matjson::Value& v) { return v.as<float>(); }));
            GEODE_UNWRAP_INTO_IF_OK(info.stroke, value.get("stroke").andThen([](const matjson::Value& v) { return v.as<float>(); }));
        }
        return geode::Ok(info);
    }

    static matjson::Value toJson(const TrailInfo& info) {
        return matjson::makeObject({
            { "blend", info.blend },
            { "tint", info.tint },
            { "show", info.show },
            { "fade", info.fade },
            { "stroke", info.stroke }
        });
    }
};

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
        if (comparison != std::strong_ordering::equal) return comparison < 0 ? -1 : 1;

        auto samePack = packID == packID2;
        if (samePack && shortName == shortName2) return 0;
        if (packID.empty() && !packID2.empty()) return -1;
        if (!packID.empty() && packID2.empty()) return 1;

        std::string_view a = samePack ? shortName : packID;
        std::string_view b = samePack ? shortName2 : packID2;

        for (size_t aIt = 0, bIt = 0; aIt < a.size() && bIt < b.size();) {
            if (isdigit(a[aIt]) && isdigit(b[bIt])) {
                auto aStart = aIt;
                for (; aIt < a.size() && isdigit(a[aIt]); aIt++);
                auto aSize = aIt - aStart;

                auto bStart = bIt;
                for (; bIt < b.size() && isdigit(b[bIt]); bIt++);
                auto bSize = bIt - bStart;

                comparison = aSize == bSize ? a.substr(aStart, aSize) <=> b.substr(bStart, bSize) : aSize <=> bSize;
            }
            else comparison = tolower(a[aIt++]) <=> tolower(b[bIt++]);

            if (comparison != std::strong_ordering::equal) return comparison < 0 ? -1 : 1;
        }

        return a.size() < b.size() ? -1 : a.size() > b.size() ? 1 : 0;
    }
};
