#include <matjson.hpp>

class Defaults {
public:
    static const matjson::Value& getTrailInfo(int id);
    static const matjson::Value& getDeathEffectInfo(int id);
    static const matjson::Value& getShipFireInfo(int id);
    static int getShipFireCount(int id);
};
