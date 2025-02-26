#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>

class LogLayer : public geode::Popup<> {
protected:
    geode::ScrollLayer* m_scrollLayer;
    CCMenuItemSpriteExtra* m_prevButton;
    CCMenuItemSpriteExtra* m_nextButton;
    int m_page;

    void page(int page);
    bool setup() override;
public:
    static LogLayer* create();

    void keyDown(cocos2d::enumKeyCodes) override;
};
