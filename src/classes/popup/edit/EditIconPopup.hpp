#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/Task.hpp>

class IconInfo;

class EditIconPopup : public geode::Popup<IconType, int, const std::string&, bool> {
protected:
    geode::EventListener<geode::Task<geode::Result<std::vector<std::filesystem::path>>>> m_listener;
    SimplePlayer* m_player;
    cocos2d::CCSprite* m_streak;
    cocos2d::CCDictionary* m_frames;
    cocos2d::CCDictionary* m_sprites;
    geode::TextInput* m_textInput;
    IconType m_iconType;
    bool m_readOnly;
    bool m_pickerOpened;

    bool setup(IconType, int, const std::string&, bool) override;

    void moveIcon(IconInfo*, const std::filesystem::path&, bool);
    void pickFile(int, int);
    void updateSprites();
    void addOrUpdateIcon(const std::string&, const std::filesystem::path&, const std::filesystem::path&);
    bool checkFrame(std::string_view);
    void saveTrail(const std::filesystem::path&);
    void saveIcon(const std::filesystem::path&, const std::filesystem::path&);
    void onClose(cocos2d::CCObject*) override;
public:
    inline static constexpr std::array lowercase = {
        "", "icon", "", "", "ship", "ball",
        "UFO", "wave", "robot", "spider", "trail",
        "death effect", "", "swing", "jetpack", "ship fire"
    };
    inline static constexpr std::array uppercase = {
        "", "Icon", "", "", "Ship", "Ball",
        "UFO", "Wave", "Robot", "Spider", "Trail",
        "Death Effect", "", "Swing", "Jetpack", "Ship Fire"
    };

    static EditIconPopup* create(IconType, int, const std::string&, bool);

    void fullClose();
    void updateGarage();

    ~EditIconPopup();
};
