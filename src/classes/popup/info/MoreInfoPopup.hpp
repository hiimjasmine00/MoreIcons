#include <Geode/ui/Popup.hpp>

class IconInfo;

class MoreInfoPopup : public geode::Popup<IconInfo*> {
protected:
    bool setup(IconInfo*) override;
public:
    static MoreInfoPopup* create(IconInfo*);
};
