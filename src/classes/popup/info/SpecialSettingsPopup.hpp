#include <Geode/ui/Popup.hpp>
#include <IconInfo.hpp>

class SpecialSettingsPopup : public geode::Popup<IconInfo*> {
protected:
    TrailInfo m_trailInfo;

    bool setup(IconInfo*) override;
public:
    static SpecialSettingsPopup* create(IconInfo*);
};
