#include "IconEditorState.hpp"
#include "../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/StringMap.hpp>

class SaveIconPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    std::array<std::filesystem::path, 3> m_pngs;
    std::array<std::filesystem::path, 3> m_plists;
    BasePopup* m_parentPopup1;
    BasePopup* m_parentPopup2;
    const IconEditorState* m_state;
    const geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>>* m_frames;
    geode::TextInput* m_nameInput;
    IconType m_iconType;

    bool init(
        BasePopup* popup1, BasePopup* popup2, IconType type,
        const IconEditorState& state, const geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>>& frames
    );
    void onSave(cocos2d::CCObject* sender);
    void saveIcon();
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static SaveIconPopup* create(
        BasePopup* popup1, BasePopup* popup2, IconType type,
        const IconEditorState& state, const geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>>& frames
    );
};
