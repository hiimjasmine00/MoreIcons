#include <Geode/binding/CCContentLayer.hpp>

class BiggerContentLayer : public CCContentLayer {
protected:
    float m_sizeOffset;
    cocos2d::CCMenuItem* m_selectedItem;

    bool init(float, float, float);
public:
    static BiggerContentLayer* create(float, float, float);

    void setPosition(const cocos2d::CCPoint&) override;
};
