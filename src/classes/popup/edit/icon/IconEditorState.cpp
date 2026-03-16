#include "IconEditorState.hpp"
#include "../../../../utils/Json.hpp"
#include <matjson/std.hpp>

using namespace geode;
using namespace std::string_literals;

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
        { "definitions"s, Value(state.definitions) },
        { "main-color"s, Value(state.mainColor) },
        { "secondary-color"s, Value(state.secondaryColor) },
        { "glow-color"s, Value(state.glowColor) }
    });
}
