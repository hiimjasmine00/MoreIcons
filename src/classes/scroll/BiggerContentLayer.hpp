#include <Geode/binding/CCContentLayer.hpp>

class BiggerContentLayer : public CCContentLayer {
protected:
    float m_sizeOffset;
    cocos2d::CCMenuItem* m_selectedItem;

    bool init(float, float, float);
    cocos2d::CCMenuItem* itemForTouch(cocos2d::CCTouch* touch);
public:
    static BiggerContentLayer* create(float, float, float);

    bool ccTouchBegan(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void ccTouchMoved(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void ccTouchEnded(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void ccTouchCancelled(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void setPosition(const cocos2d::CCPoint&) override;
};
