#include "../FrameDefinition.hpp"
#include "../../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>

class IconButton;

class SaveDeathEffectPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    std::filesystem::path m_pendingPath;
    BasePopup* m_parentPopup1;
    BasePopup* m_parentPopup2;
    const std::vector<FrameDefinition>* m_definitions;
    const std::vector<geode::Ref<cocos2d::CCSpriteFrame>>* m_frames;
    cocos2d::CCTextFieldTTF* m_nameInput;
    IconButton* m_iconButton;

    bool init(
        BasePopup* popup1, BasePopup* popup2, IconButton* iconButton,
        const std::vector<FrameDefinition>& definitions, const std::vector<geode::Ref<cocos2d::CCSpriteFrame>>& frames
    );
    void onSave(cocos2d::CCObject* sender);
    void saveIcon();
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static SaveDeathEffectPopup* create(
        BasePopup* popup1, BasePopup* popup2, IconButton* iconButton,
        const std::vector<FrameDefinition>& definitions, const std::vector<geode::Ref<cocos2d::CCSpriteFrame>>& frames
    );
};
