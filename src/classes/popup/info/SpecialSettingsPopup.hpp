#include <Geode/ui/Popup.hpp>

class IconInfo;

class SpecialSettingsPopup : public geode::Popup<IconInfo*> {
protected:
    matjson::Value m_settings;

    bool setup(IconInfo* info) override;
    void addControl(std::string_view id, const char* text, const cocos2d::CCPoint& position, float min, float max, float def, int decimals);
    void addToggle(std::string_view id, const char* text, const cocos2d::CCPoint& position, bool def);
public:
    static SpecialSettingsPopup* create(IconInfo* info);
};
