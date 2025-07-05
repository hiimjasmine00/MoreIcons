#include <Geode/ui/Popup.hpp>

class IconViewPopup : public geode::Popup<IconType, bool> {
protected:
    bool setup(IconType, bool) override;
public:
    static IconViewPopup* create(IconType, bool);
};
