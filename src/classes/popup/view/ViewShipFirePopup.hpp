#include "../BasePopup.hpp"

class IconInfo;

class ViewShipFirePopup : public BasePopup {
protected:
    std::vector<CCMenuItemSpriteExtra*> m_frameButtons;
    cocos2d::CCMenu* m_frameMenu;
    cocos2d::CCSprite* m_streak;
    cocos2d::CCSprite* m_selectSprite;
    int m_selectedFrame = 0;
    int m_page = 0;

    bool init(int id, IconInfo* info);
    CCMenuItemSpriteExtra* addFrameButton(cocos2d::CCTexture2D* texture);
    void updateState();
    void onPrev(cocos2d::CCObject* sender);
    void onNext(cocos2d::CCObject* sender);
    void onFrameSelect(cocos2d::CCObject* sender);
public:
    static ViewShipFirePopup* create(int id, IconInfo* info);
};
