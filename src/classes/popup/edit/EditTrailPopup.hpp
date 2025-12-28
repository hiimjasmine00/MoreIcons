#include "../BasePopup.hpp"
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/Task.hpp>

class EditTrailPopup : public BasePopup {
protected:
    geode::EventListener<geode::Task<geode::Result<std::filesystem::path>>> m_listener;
    BasePopup* m_parentPopup;
    cocos2d::CCSprite* m_streak;
    geode::TextInput* m_nameInput;
    bool m_hasChanged = false;

    bool init(BasePopup* popup);
    void updateWithPath(const std::filesystem::path& path);
    void saveTrail(std::filesystem::path&& path);
    void onClose(cocos2d::CCObject* sender) override;
public:
    static EditTrailPopup* create(BasePopup* popup);
};
