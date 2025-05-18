#include <Geode/ui/Popup.hpp>
#include <IconInfo.hpp>

class MoreInfoPopup : public geode::Popup<IconInfo*> {
protected:
    bool setup(IconInfo*) override;
public:
    static MoreInfoPopup* create(IconInfo*);
};
