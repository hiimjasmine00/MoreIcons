#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

class EditIconPopup;

class SaveIconPopup : public geode::Popup<EditIconPopup*, IconType, cocos2d::CCDictionary*, cocos2d::CCDictionary*> {
protected:
    EditIconPopup* m_parentPopup;
    IconType m_iconType;
    cocos2d::CCDictionary* m_definitions;
    cocos2d::CCDictionary* m_frames;
    geode::TextInput* m_nameInput;

    bool setup(EditIconPopup* popup, IconType type, cocos2d::CCDictionary* definitions, cocos2d::CCDictionary* frames) override;
    void onClose(cocos2d::CCObject* sender) override;
    bool checkFrame(std::string_view suffix);
    void saveIcon(const std::filesystem::path& stem);
    void addOrUpdateIcon(const std::string& name, const std::filesystem::path& png, const std::filesystem::path& plist);
public:
    static SaveIconPopup* create(EditIconPopup* popup, IconType type, cocos2d::CCDictionary* definitions, cocos2d::CCDictionary* frames);
};
