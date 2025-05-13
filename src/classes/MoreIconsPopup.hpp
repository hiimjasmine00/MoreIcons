#include <Geode/ui/Popup.hpp>

class MoreIconsPopup : public geode::Popup<> {
protected:
    bool setup() override;
public:
    CCMenuItemSpriteExtra* m_reloadButton;

    static MoreIconsPopup* create();
};
