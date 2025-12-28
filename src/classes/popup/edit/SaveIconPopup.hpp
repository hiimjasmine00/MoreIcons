#include "../BasePopup.hpp"
#include <Geode/ui/TextInput.hpp>

class SaveIconPopup : public BasePopup {
protected:
    BasePopup* m_parentPopup1;
    BasePopup* m_parentPopup2;
    IconType m_iconType;
    matjson::Value m_definitions;
    cocos2d::CCDictionary* m_frames;
    geode::TextInput* m_nameInput;

    bool init(BasePopup* popup1, BasePopup* popup2, IconType type, const matjson::Value& definitions, cocos2d::CCDictionary* frames);
    void onClose(cocos2d::CCObject* sender) override;
    bool checkFrame(std::string_view suffix);
    void saveIcon(std::basic_string_view<std::filesystem::path::value_type> stem);
public:
    static SaveIconPopup* create(
        BasePopup* popup1, BasePopup* popup2, IconType type, const matjson::Value& definitions, cocos2d::CCDictionary* frames
    );
};
