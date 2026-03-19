#include "../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>

class IconInfo;

class IconNamePopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    std::vector<std::pair<std::filesystem::path, std::filesystem::path>> m_pendingPaths;
    BasePopup* m_parentPopup;
    cocos2d::CCTextFieldTTF* m_nameInput;
    IconInfo* m_info;
    IconType m_iconType;

    bool init(BasePopup* popup, IconInfo* info);
    void onConfirm(cocos2d::CCObject* sender);
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static IconNamePopup* create(BasePopup* popup, IconInfo* info);
};
