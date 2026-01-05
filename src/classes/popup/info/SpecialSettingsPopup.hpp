#include "../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/binding/TextInputDelegate.hpp>

class IconInfo;

class SpecialSettingsPopup : public BasePopup, public FLAlertLayerProtocol, public TextInputDelegate {
protected:
    matjson::Value m_settings;
    IconInfo* m_info;
    IconType m_iconType;

    bool init(IconInfo* info);
    void onSave(cocos2d::CCObject* sender);
    void addControl(
        std::string_view id, const char* text, const cocos2d::CCPoint& position, float scale, float min, float max, float def, int decimals
    );
    void sliderChanged(cocos2d::CCObject* sender);
    void textChanged(CCTextInputNode* input) override;
    void addToggle(std::string_view id, const char* text, const cocos2d::CCPoint& position, float scale, bool def);
    void onToggle(cocos2d::CCObject* sender);
    void addLabel(std::string&& id, const char* text, const cocos2d::CCPoint& position);
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static SpecialSettingsPopup* create(IconInfo* info);
};
