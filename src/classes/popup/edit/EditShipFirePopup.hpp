#include "../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/utils/async.hpp>

class EditShipFirePopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    geode::async::TaskHolder<geode::Result<std::optional<std::filesystem::path>>> m_pickListener;
    geode::async::TaskHolder<geode::Result<std::vector<std::filesystem::path>>> m_pickManyListener;
    std::filesystem::path m_pendingPath;
    BasePopup* m_parentPopup;
    std::vector<CCMenuItemSpriteExtra*> m_frameButtons;
    cocos2d::CCMenu* m_frameMenu;
    cocos2d::CCSprite* m_streak;
    cocos2d::CCSprite* m_selectSprite;
    cocos2d::CCTextFieldTTF* m_nameInput;
    int m_selectedFrame = 0;
    int m_page = 0;
    bool m_hasChanged = false;

    bool init(BasePopup* popup);
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
    void updateWithPath(std::filesystem::path path, int count);
    void onSave(cocos2d::CCObject* sender);
    void saveShipFire();
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static EditShipFirePopup* create(BasePopup* popup);
};
