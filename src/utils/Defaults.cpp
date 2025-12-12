#include "Defaults.hpp"
#include <array>

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

std::array<matjson::Value, 5> shipFires = {
    matjson::makeObject({
        { "blend", true },
        { "fade", 0.0636f },
        { "stroke", 22.0f },
        { "interval", 1.0f / 32.0f },
        { "x", -8.0f },
        { "y", -3.0f }
    }),
    matjson::makeObject({
        { "blend", true },
        { "fade", 0.1278f },
        { "stroke", 28.6f },
        { "interval", 1.0f / 32.0f },
        { "x", -8.0f },
        { "y", -3.5f }
    }),
    matjson::makeObject({
        { "blend", true },
        { "fade", 0.105f },
        { "stroke", 28.6f },
        { "interval", 1.0f / 24.0f },
        { "x", -14.0f },
        { "y", -3.0f }
    }),
    matjson::makeObject({
        { "blend", true },
        { "fade", 0.09f },
        { "stroke", 18.7f },
        { "interval", 1.0f / 20.0f },
        { "x", -14.0f },
        { "y", -3.0f }
    }),
    matjson::makeObject({
        { "blend", true },
        { "fade", 0.096f },
        { "stroke", 27.0f },
        { "interval", 1.0f / 24.0f },
        { "x", -14.0f },
        { "y", -2.5f }
    })
};

matjson::Value Defaults::getShipFireInfo(int id) {
    return shipFires[id - 2];
}

int Defaults::getShipFireCount(int id) {
    constexpr std::array fireCounts = { 9, 10, 6, 16, 5 };
    return fireCounts[id - 2];
}
