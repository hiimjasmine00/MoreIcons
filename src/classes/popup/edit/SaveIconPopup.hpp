#include "../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/ui/TextInput.hpp>

class SaveIconPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    geode::Function<bool(const std::string&)> m_checkCallback;
    geode::Function<geode::Result<>(const std::string&)> m_saveCallback;
    geode::Function<void()> m_closeCallback;
    geode::TextInput* m_nameInput;
    IconType m_iconType;
    bool m_editor;

    bool init(
        IconType type, bool editor, geode::Function<bool(const std::string&)> checkCallback,
        geode::Function<geode::Result<>(const std::string&)> saveCallback, geode::Function<void()> closeCallback
    );
    void onSave(cocos2d::CCObject* sender);
    void saveIcon();
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static SaveIconPopup* create(
        IconType iconType, bool editor, geode::Function<bool(const std::string&)> checkCallback,
        geode::Function<geode::Result<>(const std::string&)> saveCallback, geode::Function<void()> closeCallback
    );
};
