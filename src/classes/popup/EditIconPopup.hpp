#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/Task.hpp>

class EditIconPopup : public geode::Popup<IconType, int, const std::string&, bool> {
protected:
    geode::EventListener<geode::Task<geode::Result<std::filesystem::path>>> m_listener;
    std::filesystem::path m_path;
    SimplePlayer* m_player;
    cocos2d::CCSprite* m_streak;
    cocos2d::CCTexture2D* m_texture;
    cocos2d::CCDictionary* m_frames;
    cocos2d::CCDictionary* m_sprites;
    cocos2d::CCArray* m_frameMenus;
    CCMenuItemSpriteExtra* m_plistButton;
    CCMenuItemSpriteExtra* m_saveButton;
    geode::TextInput* m_textInput;
    IconType m_iconType;

    bool setup(IconType, int, const std::string&, bool) override;

    void pickFile(int, int, bool);
    void updateSprites();
    void saveIcon();
    void onClose(cocos2d::CCObject* sender) override;
public:
    static EditIconPopup* create(IconType, int, const std::string&, bool);

    ~EditIconPopup();
};
