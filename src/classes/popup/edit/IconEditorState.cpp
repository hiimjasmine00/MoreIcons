#include "IconEditorState.hpp"
#include <matjson/std.hpp>

using namespace geode;

Result<FrameDefinition> matjson::Serialize<FrameDefinition>::fromJson(const matjson::Value& value) {
    if (!value.isObject()) Err("Expected object");

    FrameDefinition def;
    if (auto offsetX = value.get<float>("offset-x")) {
        def.offsetX = offsetX.unwrap();
    }
    if (auto offsetY = value.get<float>("offset-y")) {
        def.offsetY = offsetY.unwrap();
    }
    if (auto rotationX = value.get<float>("rotation-x")) {
        def.rotationX = rotationX.unwrap();
    }
    if (auto rotationY = value.get<float>("rotation-y")) {
        def.rotationY = rotationY.unwrap();
    }
    if (auto scaleX = value.get<float>("scale-x")) {
        def.scaleX = scaleX.unwrap();
    }
    if (auto scaleY = value.get<float>("scale-y")) {
        def.scaleY = scaleY.unwrap();
    }
    return Ok(def);
}

matjson::Value matjson::Serialize<FrameDefinition>::toJson(const FrameDefinition& def) {
    return makeObject({
        { "offset-x", def.offsetX },
        { "offset-y", def.offsetY },
        { "rotation-x", def.rotationX },
        { "rotation-y", def.rotationY },
        { "scale-x", def.scaleX },
        { "scale-y", def.scaleY },
    });
}

Result<IconEditorState> matjson::Serialize<IconEditorState>::fromJson(const matjson::Value& value) {
    if (!value.isObject()) Err("Expected object");

    IconEditorState state;
    if (auto definitions = value.get<std::unordered_map<std::string, FrameDefinition>>("definitions")) {
        state.definitions = definitions.unwrap();
    }
    if (auto mainColor = value.get<int>("main-color")) {
        state.mainColor = mainColor.unwrap();
    }
    if (auto secondaryColor = value.get<int>("secondary-color")) {
        state.secondaryColor = secondaryColor.unwrap();
    }
    if (auto glowColor = value.get<int>("glow-color")) {
        state.glowColor = glowColor.unwrap();
    }
    return Ok(std::move(state));
}

matjson::Value matjson::Serialize<IconEditorState>::toJson(const IconEditorState& state) {
    return makeObject({
        { "definitions", state.definitions },
        { "main-color", state.mainColor },
        { "secondary-color", state.secondaryColor },
        { "glow-color", state.glowColor },
    });
}
