#include "../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>

class IconColorPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    geode::Function<void(int)> m_callback;
    cocos2d::CCSprite* m_selectSprite;
    int m_selected;
    int m_original;

    bool init(int selected, geode::Function<void(int)> callback);
    void onColor(cocos2d::CCObject* sender);
    void onConfirm(cocos2d::CCObject* sender);
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static IconColorPopup* create(int selected, geode::Function<void(int)> callback);
};
