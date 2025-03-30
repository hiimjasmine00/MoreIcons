#include <Geode/ui/Popup.hpp>
#include <MoreIcons.hpp>

class MoreInfoPopup : public geode::Popup<IconInfo*> {
protected:
    bool setup(IconInfo*) override;
public:
    static MoreInfoPopup* create(IconInfo*);
};
