#include "../BasePopup.hpp"
#include <Geode/ui/TextInput.hpp>

class IconInfo;
class MoreInfoPopup;

class IconNamePopup : public BasePopup {
protected:
    geode::TextInput* m_nameInput;
    IconInfo* m_info;
    IconType m_iconType;

    bool init(MoreInfoPopup* popup, IconInfo* info);
    void onClose(cocos2d::CCObject* sender) override;
public:
    static IconNamePopup* create(MoreInfoPopup* popup, IconInfo* info);
};
