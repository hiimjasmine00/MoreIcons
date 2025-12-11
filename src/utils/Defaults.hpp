#include <matjson.hpp>

class Defaults {
public:
    static matjson::Value getTrailInfo(int id);
    static matjson::Value getDeathEffectInfo(int id);
    static matjson::Value getShipFireInfo(int id);
};
