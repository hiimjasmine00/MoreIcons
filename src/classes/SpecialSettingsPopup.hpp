#include <Geode/ui/Popup.hpp>
#include <MoreIcons.hpp>

class SpecialSettingsPopup : public geode::Popup<IconInfo*> {
protected:
    float m_fadeTime;
    float m_strokeWidth;

    bool setup(IconInfo*) override;
public:
    static SpecialSettingsPopup* create(IconInfo*);
};
