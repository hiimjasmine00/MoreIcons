#include <cocos2d.h>
#include <matjson.hpp>

class SpecialControlData : public cocos2d::CCObject {
public:
    matjson::Value* value;
    float min;
    float max;
    float def;
    int decimals;
    int exponent;

    static SpecialControlData* create(matjson::Value& value, float min, float max, float def, int decimals, int exponent);
};
