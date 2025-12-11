#include <Geode/ui/Popup.hpp>

class IconInfo;

class SpecialSettingsPopup : public geode::Popup<IconInfo*> {
protected:
    float m_fade;
    float m_stroke;
    bool m_blend;
    bool m_tint;
    bool m_show;

    bool setup(IconInfo*) override;
public:
    static SpecialSettingsPopup* create(IconInfo*);
};
