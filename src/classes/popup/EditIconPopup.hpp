#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/Task.hpp>

class MoreIconsPopup;

class EditIconPopup : public geode::Popup<MoreIconsPopup*, IconType> {
protected:
    geode::EventListener<geode::Task<geode::Result<std::vector<std::filesystem::path>>>> m_listener;
    MoreIconsPopup* m_parentPopup;
    SimplePlayer* m_player;
    cocos2d::CCSprite* m_streak;
    geode::Ref<cocos2d::CCDictionary> m_frames;
    geode::Ref<cocos2d::CCDictionary> m_sprites;
    geode::TextInput* m_textInput;
    IconType m_iconType;
    bool m_pickerOpened;

    bool setup(MoreIconsPopup*, IconType) override;

    void pickFile(int, std::string_view);
    void updateSprites();
    void addOrUpdateIcon(const std::string&, const std::filesystem::path&, const std::filesystem::path&);
    bool checkFrame(std::string_view);
    void saveTrail(const std::filesystem::path&);
    void saveIcon(const std::filesystem::path&, const std::filesystem::path&);
    void onClose(cocos2d::CCObject*) override;
public:
    static EditIconPopup* create(MoreIconsPopup*, IconType);
};
