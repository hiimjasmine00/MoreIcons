#include <Geode/ui/Popup.hpp>

class IconInfo;

class ViewIconPopup : public geode::Popup<IconType, int, IconInfo*> {
protected:
    bool setup(IconType, int, IconInfo*) override;
public:
    static ViewIconPopup* create(IconType, int, IconInfo*);
};
