#include "../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/async.hpp>

class EditTrailPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    geode::async::TaskHolder<geode::Result<std::optional<std::filesystem::path>>> m_listener;
    std::filesystem::path m_pendingPath;
    BasePopup* m_parentPopup;
    cocos2d::CCSprite* m_streak;
    geode::TextInput* m_nameInput;
    bool m_hasChanged = false;

    bool init(BasePopup* popup);
    void onPNG(cocos2d::CCObject* sender);
    void onPreset(cocos2d::CCObject* sender);
    void onSave(cocos2d::CCObject* sender);
    void updateWithPath(const std::filesystem::path& path);
    void saveTrail();
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static EditTrailPopup* create(BasePopup* popup);
};
