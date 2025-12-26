#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

class EditIconPopup;

class SaveIconPopup : public geode::Popup<EditIconPopup*, IconType, const matjson::Value&, cocos2d::CCDictionary*> {
protected:
    EditIconPopup* m_parentPopup;
    IconType m_iconType;
    matjson::Value m_definitions;
    cocos2d::CCDictionary* m_frames;
    geode::TextInput* m_nameInput;

    bool setup(EditIconPopup* popup, IconType type, const matjson::Value& definitions, cocos2d::CCDictionary* frames) override;
    void onClose(cocos2d::CCObject* sender) override;
    bool checkFrame(std::string_view suffix);
    void saveIcon(std::basic_string_view<std::filesystem::path::value_type> stem);
public:
    static SaveIconPopup* create(EditIconPopup* popup, IconType type, const matjson::Value& definitions, cocos2d::CCDictionary* frames);
};
