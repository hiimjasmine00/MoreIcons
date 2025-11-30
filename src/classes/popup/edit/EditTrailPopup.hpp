#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/Task.hpp>

class MoreIconsPopup;

class EditTrailPopup : public geode::Popup<MoreIconsPopup*> {
protected:
    geode::EventListener<geode::Task<geode::Result<std::filesystem::path>>> m_listener;
    MoreIconsPopup* m_parentPopup;
    cocos2d::CCSprite* m_streak;
    geode::TextInput* m_nameInput;
    bool m_hasChanged = false;

    bool setup(MoreIconsPopup* popup) override;
    void addOrUpdateIcon(const std::string& name, const std::filesystem::path& path);
    void saveTrail(const std::filesystem::path& path);
    void onClose(cocos2d::CCObject* sender) override;
public:
    static EditTrailPopup* create(MoreIconsPopup* popup);
};
