#include "../BasePopup.hpp"
#include "../../misc/SimpleIcon.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/utils/async.hpp>

class IconInfo;

class MoreInfoPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    geode::async::TaskHolder<geode::Result<std::optional<std::filesystem::path>>> m_listener;
    std::filesystem::path m_pendingPath;
    SimpleIcon* m_icon;
    IconInfo* m_info;
    bool m_toggled;

    bool init(IconInfo* info);
    void onIcon(cocos2d::CCObject* sender);
    void onSwapIcon(cocos2d::CCObject* sender);
    void onSettings(cocos2d::CCObject* sender);
    void onConvert(cocos2d::CCObject* sender);
    void onRename(cocos2d::CCObject* sender);
    void onTrash(cocos2d::CCObject* sender);
    void moveIcon(bool trash);
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static MoreInfoPopup* create(IconInfo* info);
};
