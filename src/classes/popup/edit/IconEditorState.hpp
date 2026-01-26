#pragma once
#include <Geode/utils/StringMap.hpp>

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

struct IconEditorState {
    geode::utils::StringMap<FrameDefinition> definitions;
    int mainColor = 12;
    int secondaryColor = 12;
    int glowColor = 12;
};

template <>
struct matjson::Serialize<IconEditorState> {
    static geode::Result<IconEditorState> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const IconEditorState& state);
};
