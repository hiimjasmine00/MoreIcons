#pragma once
#include <matjson.hpp>

struct FrameDefinition {
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};

template <>
struct matjson::Serialize<FrameDefinition> {
    static geode::Result<FrameDefinition> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const FrameDefinition& def);
};
