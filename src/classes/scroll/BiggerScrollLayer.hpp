#include <Geode/binding/CCScrollLayerExt.hpp>
#include <Geode/utils/cocos.hpp>

class BiggerScrollLayer : public CCScrollLayerExt {
protected:
    geode::Ref<cocos2d::extension::CCScale9Sprite> m_stencil;

    BiggerScrollLayer(float, float, float, float);
public:
    static BiggerScrollLayer* create(float, float, float = 0.0f, float = 0.0f);

    bool ccTouchBegan(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void ccTouchMoved(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void ccTouchEnded(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void ccTouchCancelled(cocos2d::CCTouch*, cocos2d::CCEvent*) override;
    void onEnter() override;
    void onEnterTransitionDidFinish() override;
    void onExit() override;
    void onExitTransitionDidStart() override;
    void scrollToTop();
    void scrollWheel(float, float) override;
    void visit() override;
};
