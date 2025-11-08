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
            if (auto blend = value.get<bool>("blend")) info.blend = blend.unwrap();
            if (auto tint = value.get<bool>("tint")) info.tint = tint.unwrap();
            if (auto show = value.get<bool>("show")) info.show = show.unwrap();
            if (auto fade = value.get<float>("fade")) info.fade = fade.unwrap();
            if (auto stroke = value.get<float>("stroke")) info.stroke = stroke.unwrap();
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
