#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/utils/async.hpp>

class IconButton : public CCMenuItemSpriteExtra {
protected:
    geode::async::TaskHolder<geode::Result<std::optional<asp::fs::Path>>> m_listener;
    cocos2d::CCSprite* m_unselectedSprite;
    cocos2d::CCLabelBMFont* m_plusLabel;

    bool init() override;
    void activate() override;
public:
    static IconButton* create();

    asp::fs::Path saveIcon(asp::fs::PathView path);
    void setIcon(asp::fs::PathView path);
    void resetIcon();
    cocos2d::CCTexture2D* getIconTexture();
};
