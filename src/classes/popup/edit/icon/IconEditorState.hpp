#pragma once
#include "../FrameDefinition.hpp"
#include <Geode/utils/StringMap.hpp>

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
