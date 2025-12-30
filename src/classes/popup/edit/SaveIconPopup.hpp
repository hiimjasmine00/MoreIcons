#include "IconEditorState.hpp"
#include "../BasePopup.hpp"
#include <Geode/ui/TextInput.hpp>

class SaveIconPopup : public BasePopup {
protected:
    BasePopup* m_parentPopup1;
    BasePopup* m_parentPopup2;
    IconType m_iconType;
    const std::unordered_map<std::string, FrameDefinition>* m_definitions;
    const std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>>* m_frames;
    geode::TextInput* m_nameInput;

    bool init(
        BasePopup* popup1, BasePopup* popup2, IconType type,
        const std::unordered_map<std::string, FrameDefinition>& definitions,
        const std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>>& frames
    );
    void onClose(cocos2d::CCObject* sender) override;
    bool checkFrame(const std::string& suffix);
    void saveIcon(std::basic_string_view<std::filesystem::path::value_type> stem);
public:
    static SaveIconPopup* create(
        BasePopup* popup1, BasePopup* popup2, IconType type,
        const std::unordered_map<std::string, FrameDefinition>& definitions,
        const std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>>& frames
    );
};
