#include <Geode/ui/Popup.hpp>

class IconInfo;

class ViewIconPopup : public geode::Popup<IconType, int, IconInfo*> {
protected:
    bool setup(IconType type, int id, IconInfo* info) override;
public:
    static ViewIconPopup* create(IconType type, int id, IconInfo* info);
};
