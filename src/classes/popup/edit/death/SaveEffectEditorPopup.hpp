#include "../FrameDefinition.hpp"
#include "../../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/utils/StringMap.hpp>

class IconButton;

class SaveEffectEditorPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    const std::vector<FrameDefinition>* m_definitions;
    std::filesystem::path m_pendingPath;
    geode::Function<void()> m_callback;
    const std::vector<geode::Ref<cocos2d::CCSpriteFrame>>* m_frames;
    IconButton* m_iconButton;
    cocos2d::CCTextFieldTTF* m_nameInput;

    bool init(
        const std::vector<FrameDefinition>& definitions,
        const std::vector<geode::Ref<cocos2d::CCSpriteFrame>>& frames, IconButton* iconButton, geode::Function<void()> callback
    );
    void onSave(cocos2d::CCObject* sender);
    void saveEditor();
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static SaveEffectEditorPopup* create(
        const std::vector<FrameDefinition>& definitions,
        const std::vector<geode::Ref<cocos2d::CCSpriteFrame>>& frames, IconButton* iconButton, geode::Function<void()> callback
    );
};
