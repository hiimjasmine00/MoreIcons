#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

class IconInfo;

class IconNamePopup : public geode::Popup<IconInfo*> {
protected:
    geode::TextInput* m_nameInput;
    IconType m_iconType;

    bool setup(IconInfo*) override;

    void onClose(cocos2d::CCObject*) override;
public:
    static IconNamePopup* create(IconInfo*);
};
