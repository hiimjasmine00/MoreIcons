#pragma once
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

    TrailInfo& operator=(const TrailInfo& other) {
        blend = other.blend;
        tint = other.tint;
        show = other.show;
        fade = other.fade;
        stroke = other.stroke;
        return *this;
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
