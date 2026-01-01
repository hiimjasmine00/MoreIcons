#include "IconEditorState.hpp"
#include <matjson/std.hpp>

using namespace geode;
using namespace std::string_literals;

Result<FrameDefinition> matjson::Serialize<FrameDefinition>::fromJson(const matjson::Value& value) {
    if (!value.isObject()) Err("Expected object");

    FrameDefinition def;
    def.offsetX = value["offset-x"].asDouble().unwrapOr(0.0);
    def.offsetY = value["offset-y"].asDouble().unwrapOr(0.0);
    def.rotationX = value["rotation-x"].asDouble().unwrapOr(0.0);
    def.rotationY = value["rotation-y"].asDouble().unwrapOr(0.0);
    def.scaleX = value["scale-x"].asDouble().unwrapOr(1.0);
    def.scaleY = value["scale-y"].asDouble().unwrapOr(1.0);
    return Ok(def);
}

matjson::Value matjson::Serialize<FrameDefinition>::toJson(const FrameDefinition& def) {
    return makeObject({
        { "offset-x"s, matjson::Value(def.offsetX) },
        { "offset-y"s, matjson::Value(def.offsetY) },
        { "rotation-x"s, matjson::Value(def.rotationX) },
        { "rotation-y"s, matjson::Value(def.rotationY) },
        { "scale-x"s, matjson::Value(def.scaleX) },
        { "scale-y"s, matjson::Value(def.scaleY) },
    });
}

Result<IconEditorState> matjson::Serialize<IconEditorState>::fromJson(const matjson::Value& value) {
    if (!value.isObject()) Err("Expected object");

    IconEditorState state;
    state.definitions = value["definitions"].as<std::unordered_map<std::string, FrameDefinition>>().unwrapOrDefault();
    state.mainColor = value["main-color"].asInt().unwrapOr(12);
    state.secondaryColor = value["secondary-color"].asInt().unwrapOr(12);
    state.glowColor = value["glow-color"].asInt().unwrapOr(12);
    return Ok(std::move(state));
}

matjson::Value matjson::Serialize<IconEditorState>::toJson(const IconEditorState& state) {
    return makeObject({
        { "definitions"s, matjson::Value(state.definitions) },
        { "main-color"s, matjson::Value(state.mainColor) },
        { "secondary-color"s, matjson::Value(state.secondaryColor) },
        { "glow-color"s, matjson::Value(state.glowColor) },
    });
}
