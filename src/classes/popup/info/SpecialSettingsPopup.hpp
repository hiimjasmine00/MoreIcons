#include "../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>

class IconInfo;

class SpecialSettingsPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    matjson::Value m_settings;
    IconInfo* m_info;
    IconType m_iconType;
    bool m_hasChanged = false;

    bool init(IconInfo* info);
    void checkDefaults(const matjson::Value& defaultInfo);
    void onSave(cocos2d::CCObject* sender);
    void addControl(
        std::string_view id, const char* text, const cocos2d::CCPoint& position, float scale, float min, float max, float def, int decimals
    );
    void addToggle(std::string_view id, const char* text, const cocos2d::CCPoint& position, float scale, bool def);
    void onToggle(cocos2d::CCObject* sender);
    void addLabel(std::string&& id, const char* text, const cocos2d::CCPoint& position);
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static SpecialSettingsPopup* create(IconInfo* info);
};
