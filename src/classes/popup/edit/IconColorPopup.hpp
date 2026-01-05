#include "../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <std23/move_only_function.h>

class IconColorPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    std23::move_only_function<void(int)> m_callback;
    cocos2d::CCSprite* m_selectSprite;
    int m_selected;
    int m_original;

    bool init(int selected, std23::move_only_function<void(int)> callback);
    void onColor(cocos2d::CCObject* sender);
    void onConfirm(cocos2d::CCObject* sender);
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static IconColorPopup* create(int selected, std23::move_only_function<void(int)> callback);
};
