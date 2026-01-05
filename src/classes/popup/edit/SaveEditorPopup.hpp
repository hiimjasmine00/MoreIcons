#include "IconEditorState.hpp"
#include "../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/ui/TextInput.hpp>
#include <std23/move_only_function.h>

class SaveEditorPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    const IconEditorState* m_state;
    std::filesystem::path m_pendingPath;
    std23::move_only_function<void()> m_callback;
    const std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>>* m_frames;
    geode::TextInput* m_nameInput;
    IconType m_iconType;

    bool init(
        IconType type, const IconEditorState& state,
        const std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>>& frames, std23::move_only_function<void()> callback
    );
    void onSave(cocos2d::CCObject* sender);
    void saveEditor();
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static SaveEditorPopup* create(
        IconType type, const IconEditorState& state,
        const std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>>& frames, std23::move_only_function<void()> callback
    );
};
