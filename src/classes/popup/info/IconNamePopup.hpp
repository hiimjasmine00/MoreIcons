#include "../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/ui/TextInput.hpp>

class IconInfo;

class IconNamePopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    std::vector<std::pair<asp::fs::Path, asp::fs::Path>> m_pendingPaths;
    geode::TextInput* m_nameInput;
    IconInfo* m_info;
    IconType m_iconType;

    bool init(IconInfo* info);
    void onConfirm(cocos2d::CCObject* sender);
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static IconNamePopup* create(IconInfo* info);
};
