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
            auto blend = value.get("blend").andThen([](const matjson::Value& v) {
                return v.asBool();
            });
            if (blend.isOk()) info.blend = blend.unwrap();

            auto tint = value.get("tint").andThen([](const matjson::Value& v) {
                return v.asBool();
            });
            if (tint.isOk()) info.tint = tint.unwrap();

            auto show = value.get("show").andThen([](const matjson::Value& v) {
                return v.asBool();
            });
            if (show.isOk()) info.show = show.unwrap();

            auto fade = value.get("fade").andThen([](const matjson::Value& v) {
                return v.asDouble();
            });
            if (fade.isOk()) info.fade = fade.unwrap();

            auto stroke = value.get("stroke").andThen([](const matjson::Value& v) {
                return v.asDouble();
            });
            if (stroke.isOk()) info.stroke = stroke.unwrap();
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
