#include "../../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/utils/async.hpp>

class IconButton;

class EditShipFirePopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    geode::async::TaskHolder<geode::Result<std::optional<asp::fs::Path>>> m_pickListener;
    geode::async::TaskHolder<geode::Result<std::vector<asp::fs::Path>>> m_pickManyListener;
    asp::fs::Path m_pendingPath;
    std::vector<CCMenuItemSpriteExtra*> m_frameButtons;
    cocos2d::CCMenu* m_frameMenu;
    cocos2d::CCSprite* m_streak;
    cocos2d::CCSprite* m_selectSprite;
    IconButton* m_iconButton;
    int m_selectedFrame = 0;
    int m_page = 0;
    bool m_hasChanged = false;

    bool init() override;
    CCMenuItemSpriteExtra* addFrameButton(cocos2d::CCTexture2D* texture);
    void updateState();
    void onPrev(cocos2d::CCObject* sender);
    void onNext(cocos2d::CCObject* sender);
    void onFrameSelect(cocos2d::CCObject* sender);
    void onFrameAdd(cocos2d::CCObject* sender);
    void onFrameImport(cocos2d::CCObject* sender);
    void onFramePreset(cocos2d::CCObject* sender);
    void onFrameRemove(cocos2d::CCObject* sender);
    void onImport(cocos2d::CCObject* sender);
    void onPreset(cocos2d::CCObject* sender);
    void updateWithPath(asp::fs::Path path, int count);
    void onSave(cocos2d::CCObject* sender);
    geode::Result<> saveShipFire(const std::string& name);
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static EditShipFirePopup* create();
};
