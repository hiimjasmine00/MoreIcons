#include "../BasePopup.hpp"

class IconInfo;

class SpecialSettingsPopup : public BasePopup {
protected:
    matjson::Value m_settings;

    bool init(IconInfo* info);
    void addControl(
        std::string_view id, const char* text, const cocos2d::CCPoint& position, float scale, float min, float max, float def, int decimals
    );
    void addToggle(std::string_view id, const char* text, const cocos2d::CCPoint& position, float scale, bool def);
    void addLabel(std::string&& id, const char* text, const cocos2d::CCPoint& position);
public:
    static SpecialSettingsPopup* create(IconInfo* info);
};
