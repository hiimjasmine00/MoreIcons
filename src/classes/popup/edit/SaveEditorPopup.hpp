#include "IconEditorState.hpp"
#include "../BasePopup.hpp"
#include <Geode/ui/TextInput.hpp>
#include <std23/move_only_function.h>

class SaveEditorPopup : public BasePopup {
protected:
    std23::move_only_function<void()> m_callback;
    IconType m_iconType;
    IconEditorState m_state;
    cocos2d::CCDictionary* m_frames;
    geode::TextInput* m_nameInput;

    bool init(IconType type, const IconEditorState& state, cocos2d::CCDictionary* frames, std23::move_only_function<void()> callback);
    void onClose(cocos2d::CCObject* sender) override;
    void saveEditor(const std::filesystem::path& directory);
public:
    static SaveEditorPopup* create(
        IconType type, const IconEditorState& state, cocos2d::CCDictionary* frames, std23::move_only_function<void()> callback
    );
};
