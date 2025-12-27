#include <matjson.hpp>

namespace Defaults {
    const matjson::Value& getTrailInfo(int id);
    const matjson::Value& getDeathEffectInfo(int id);
    const matjson::Value& getShipFireInfo(int id);
    int getShipFireCount(int id);
};
