#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

class IconInfo;
class MoreInfoPopup;

class IconNamePopup : public geode::Popup<MoreInfoPopup*, IconInfo*> {
protected:
    geode::TextInput* m_nameInput;
    IconType m_iconType;

    bool setup(MoreInfoPopup*, IconInfo*) override;

    void onClose(cocos2d::CCObject*) override;
public:
    static IconNamePopup* create(MoreInfoPopup*, IconInfo*);
};
