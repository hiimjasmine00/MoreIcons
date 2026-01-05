#include "SpecialControlData.hpp"

SpecialControlData* SpecialControlData::create(matjson::Value& value, float min, float max, float def, int decimals, int exponent) {
    auto ret = new SpecialControlData();
    ret->value = &value;
    ret->min = min;
    ret->max = max;
    ret->def = def;
    ret->decimals = decimals;
    ret->exponent = exponent;
    ret->autorelease();
    return ret;
}
