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
        return name == other.name && type == other.type;
    }

    std::strong_ordering operator<=>(const IconInfo& other) const {
        if (type != other.type) return type <=> other.type;
        if (name == other.name) return std::strong_ordering::equal;
        if (packID.empty() && !other.packID.empty()) return std::strong_ordering::less;
        if (!packID.empty() && other.packID.empty()) return std::strong_ordering::greater;

        auto differentPack = packID != other.packID;
        std::string_view a = differentPack ? packID : shortName;
        std::string_view b = differentPack ? other.packID : other.shortName;

        for (size_t aIt = 0, bIt = 0; aIt < a.size() && bIt < b.size();) {
            if (isdigit(a[aIt]) && isdigit(b[bIt])) {
                auto aStart = aIt;
                auto bStart = bIt;
                for (; aIt < a.size() && isdigit(a[aIt]); aIt++);
                for (; bIt < b.size() && isdigit(b[bIt]); bIt++);
                auto aNum = a.substr(aStart, aIt - aStart);
                auto bNum = b.substr(bStart, bIt - bStart);
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
