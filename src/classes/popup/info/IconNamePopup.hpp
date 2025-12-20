#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

class IconInfo;
class MoreInfoPopup;

class IconNamePopup : public geode::Popup<MoreInfoPopup*, IconInfo*> {
protected:
    geode::TextInput* m_nameInput;
    IconInfo* m_info;
    IconType m_iconType;

    bool setup(MoreInfoPopup* popup, IconInfo* info) override;
    void onClose(cocos2d::CCObject* sender) override;
public:
    static IconNamePopup* create(MoreInfoPopup* popup, IconInfo* info);
};
