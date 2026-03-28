#include "../../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/utils/async.hpp>

class IconButton;

class EditTrailPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    geode::async::TaskHolder<geode::Result<std::optional<std::filesystem::path>>> m_listener;
    std::filesystem::path m_pendingPath;
    cocos2d::CCSprite* m_streak;
    IconButton* m_iconButton;
    bool m_hasChanged = false;

    bool init() override;
    void onPNG(cocos2d::CCObject* sender);
    void onPreset(cocos2d::CCObject* sender);
    void onSave(cocos2d::CCObject* sender);
    void updateWithPath(const std::filesystem::path& path);
    geode::Result<> saveTrail(const gd::string& name);
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static EditTrailPopup* create();
};
