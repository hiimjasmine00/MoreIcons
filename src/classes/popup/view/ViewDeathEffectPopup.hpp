#include "../BasePopup.hpp"

class IconInfo;

class ViewDeathEffectPopup : public BasePopup {
protected:
    std::vector<CCMenuItemSpriteExtra*> m_frameButtons;
    std::vector<geode::Ref<cocos2d::CCSpriteFrame>> m_frames;
    cocos2d::CCMenu* m_frameMenu;
    cocos2d::CCSprite* m_previewSprite;
    cocos2d::CCSprite* m_selectSprite;
    int m_selectedFrame = 0;
    int m_page = 0;

    bool init(int id, IconInfo* info);
    CCMenuItemSpriteExtra* addFrameButton(cocos2d::CCSpriteFrame* frame);
    void updateState();
    void onPrev(cocos2d::CCObject* sender);
    void onNext(cocos2d::CCObject* sender);
    void onFrameSelect(cocos2d::CCObject* sender);
public:
    static ViewDeathEffectPopup* create(int id, IconInfo* info);
};
