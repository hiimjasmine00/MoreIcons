#include <Geode/ui/Popup.hpp>

class IconViewPopup : public geode::Popup<IconType, bool> {
protected:
    bool setup(IconType type, bool custom) override;
public:
    static IconViewPopup* create(IconType type, bool custom);
};
