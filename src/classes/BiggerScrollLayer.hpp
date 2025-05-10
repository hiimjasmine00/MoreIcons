#include <Geode/binding/CCScrollLayerExt.hpp>

class BiggerScrollLayer : public CCScrollLayerExt {
protected:
    float m_sizeOffset;

    BiggerScrollLayer(float, float, float);
public:
    static BiggerScrollLayer* create(float, float, float);

    bool ccTouchBegan(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void ccTouchMoved(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void ccTouchEnded(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void ccTouchCancelled(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void scrollToTop();
    void scrollWheel(float, float) override;
    void visit() override;
};
