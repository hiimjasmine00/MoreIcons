#include "IconEditorState.hpp"

using namespace geode;

Result<IconEditorState> matjson::Serialize<IconEditorState>::fromJson(const matjson::Value& value) {
    if (!value.isObject()) Err("Expected object");

    IconEditorState state;
    if (auto definitions = value.get("definitions")) state.definitions = definitions.unwrap();
    if (auto mainColor = value.get<int>("main-color")) state.mainColor = mainColor.unwrap();
    if (auto secondaryColor = value.get<int>("secondary-color")) state.secondaryColor = secondaryColor.unwrap();
    if (auto glowColor = value.get<int>("glow-color")) state.glowColor = glowColor.unwrap();
    return Ok(std::move(state));
}

matjson::Value matjson::Serialize<IconEditorState>::toJson(const IconEditorState& state) {
    return matjson::makeObject({
        { "definitions", state.definitions },
        { "main-color", state.mainColor },
        { "secondary-color", state.secondaryColor },
        { "glow-color", state.glowColor },
    });
}
