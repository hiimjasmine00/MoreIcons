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
    bool checkFrame(const std::string& suffix);
    void saveIcon(const std::filesystem::path::string_type& stem);
    void addOrUpdateIcon(const std::string& name, const std::filesystem::path& png, const std::filesystem::path& plist);
public:
    static SaveIconPopup* create(EditIconPopup* popup, IconType type, const matjson::Value& definitions, cocos2d::CCDictionary* frames);
};
