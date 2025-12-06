#include "IconEditorState.hpp"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <std23/move_only_function.h>

class SaveEditorPopup : public geode::Popup<IconType, const IconEditorState&, cocos2d::CCDictionary*, std23::move_only_function<void()>> {
protected:
    std23::move_only_function<void()> m_callback;
    IconType m_iconType;
    IconEditorState m_state;
    cocos2d::CCDictionary* m_frames;
    geode::TextInput* m_nameInput;

    bool setup(IconType type, const IconEditorState& state, cocos2d::CCDictionary* frames, std23::move_only_function<void()> callback) override;
    void onClose(cocos2d::CCObject* sender) override;
    void saveEditor(const std::filesystem::path& directory);
public:
    static SaveEditorPopup* create(
        IconType type, const IconEditorState& state, cocos2d::CCDictionary* frames, std23::move_only_function<void()> callback
    );
};
