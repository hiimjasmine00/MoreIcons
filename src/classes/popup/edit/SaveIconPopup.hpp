#include "../BasePopup.hpp"
#include "IconEditorState.hpp"
#include <Geode/ui/TextInput.hpp>

class SaveIconPopup : public BasePopup {
protected:
    BasePopup* m_parentPopup1;
    BasePopup* m_parentPopup2;
    IconType m_iconType;
    StringMap<FrameDefinition> m_definitions;
    StringMap<geode::Ref<cocos2d::CCSpriteFrame>> m_frames;
    geode::TextInput* m_nameInput;

    bool init(
        BasePopup* popup1, BasePopup* popup2, IconType type,
        const StringMap<FrameDefinition>& definitions, const StringMap<geode::Ref<cocos2d::CCSpriteFrame>>& frames
    );
    void onClose(cocos2d::CCObject* sender) override;
    bool checkFrame(std::string_view suffix);
    void saveIcon(std::basic_string_view<std::filesystem::path::value_type> stem);
public:
    static SaveIconPopup* create(
        BasePopup* popup1, BasePopup* popup2, IconType type,
        const StringMap<FrameDefinition>& definitions, const StringMap<geode::Ref<cocos2d::CCSpriteFrame>>& frames
    );
};
