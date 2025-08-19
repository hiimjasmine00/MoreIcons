#include <Geode/ui/Popup.hpp>
#include <TrailInfo.hpp>

class IconInfo;

class SpecialSettingsPopup : public geode::Popup<IconInfo*> {
protected:
    TrailInfo m_trailInfo;

    bool setup(IconInfo*) override;
public:
    static SpecialSettingsPopup* create(IconInfo*);
};
