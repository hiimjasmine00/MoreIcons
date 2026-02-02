#include "IconEditorState.hpp"
#include "../../../utils/Json.hpp"
#include <matjson/std.hpp>

using namespace geode;
using namespace std::string_literals;

Result<FrameDefinition> matjson::Serialize<FrameDefinition>::fromJson(const matjson::Value& value) {
    if (!value.isObject()) Err("Expected object");

    FrameDefinition def;
    def.offsetX = Json::get(value, "offset-x", 0.0f);
    def.offsetY = Json::get(value, "offset-y", 0.0f);
    def.rotationX = Json::get(value, "rotation-x", 0.0f);
    def.rotationY = Json::get(value, "rotation-y", 0.0f);
    def.scaleX = Json::get(value, "scale-x", 1.0f);
    def.scaleY = Json::get(value, "scale-y", 1.0f);
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
    state.definitions = Json::get<utils::StringMap<FrameDefinition>>(value, "definitions");
    state.mainColor = Json::get(value, "main-color", 12);
    state.secondaryColor = Json::get(value, "secondary-color", 12);
    state.glowColor = Json::get(value, "glow-color", 12);
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
