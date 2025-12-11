#include "Defaults.hpp"

std::array<matjson::Value, 7> trails = {
    matjson::makeObject({
        { "blend", true },
        { "tint", true },
        { "show", false },
        { "fade", 0.3f },
        { "stroke", 10.0f }
    }),
    matjson::makeObject({
        { "blend", true },
        { "tint", false },
        { "show", false },
        { "fade", 0.3f },
        { "stroke", 14.0f }
    }),
    matjson::makeObject({
        { "blend", true },
        { "tint", true },
        { "show", false },
        { "fade", 0.3f },
        { "stroke", 8.5f }
    }),
    matjson::makeObject({
        { "blend", true },
        { "tint", true },
        { "show", false },
        { "fade", 0.4f },
        { "stroke", 10.0f }
    }),
    matjson::makeObject({
        { "blend", true },
        { "tint", true },
        { "show", true },
        { "fade", 0.6f },
        { "stroke", 5.0f }
    }),
    matjson::makeObject({
        { "blend", true },
        { "tint", true },
        { "show", true },
        { "fade", 1.0f },
        { "stroke", 3.0f }
    }),
    matjson::makeObject({
        { "blend", true },
        { "tint", false },
        { "show", false },
        { "fade", 0.3f },
        { "stroke", 14.0f }
    })
};

matjson::Value Defaults::getTrailInfo(int id) {
    return trails[id - 1];
}
