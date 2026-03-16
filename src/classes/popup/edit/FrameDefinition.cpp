#include "FrameDefinition.hpp"
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
        { "offset-x"s, Value(def.offsetX) },
        { "offset-y"s, Value(def.offsetY) },
        { "rotation-x"s, Value(def.rotationX) },
        { "rotation-y"s, Value(def.rotationY) },
        { "scale-x"s, Value(def.scaleX) },
        { "scale-y"s, Value(def.scaleY) }
    });
}
