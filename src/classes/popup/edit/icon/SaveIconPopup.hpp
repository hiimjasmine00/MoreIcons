#include "IconEditorState.hpp"
#include "../../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/utils/StringMap.hpp>

class SaveIconPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    std::array<std::filesystem::path, 3> m_pngs;
    std::array<std::filesystem::path, 3> m_plists;
    const IconEditorState* m_state;
    const geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>>* m_frames;
    cocos2d::CCTextFieldTTF* m_nameInput;
    IconType m_iconType;

    bool init(
        IconType type, const IconEditorState& state, const geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>>& frames
    );
    void onSave(cocos2d::CCObject* sender);
    void saveIcon();
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static SaveIconPopup* create(
        IconType type, const IconEditorState& state, const geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>>& frames
    );
};
