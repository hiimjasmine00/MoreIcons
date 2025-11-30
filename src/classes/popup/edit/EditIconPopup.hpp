#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/Task.hpp>

class MoreIconsPopup;

class EditIconPopup : public geode::Popup<MoreIconsPopup*, IconType> {
protected:
    MoreIconsPopup* m_parentPopup;
    geode::EventListener<geode::Task<geode::Result<std::filesystem::path>>> m_listener;
    std::filesystem::path m_selectedPNG;
    std::filesystem::path m_selectedPlist;
    geode::Ref<cocos2d::CCArray> m_pages;
    geode::Ref<cocos2d::CCDictionary> m_definitions;
    geode::Ref<cocos2d::CCDictionary> m_pieces;
    geode::Ref<cocos2d::CCDictionary> m_frames;
    geode::Ref<SimplePlayer> m_player;
    Slider* m_offsetXSlider;
    Slider* m_offsetYSlider;
    Slider* m_rotationXSlider;
    Slider* m_rotationYSlider;
    Slider* m_scaleXSlider;
    Slider* m_scaleYSlider;
    geode::TextInput* m_offsetXInput;
    geode::TextInput* m_offsetYInput;
    geode::TextInput* m_rotationXInput;
    geode::TextInput* m_rotationYInput;
    geode::TextInput* m_scaleXInput;
    geode::TextInput* m_scaleYInput;
    std::string_view m_suffix;
    cocos2d::CCArray* m_targets;
    cocos2d::CCArray* m_descriptions;
    cocos2d::CCSprite* m_selectSprite;
    cocos2d::CCMenu* m_pieceMenu;
    IconType m_iconType;
    int m_page = 0;
    float m_offsetX = 0.0f;
    float m_offsetY = 0.0f;
    float m_rotationX = 0.0f;
    float m_rotationY = 0.0f;
    float m_scaleX = 1.0f;
    float m_scaleY = 1.0f;
    bool m_hasChanged = false;

    bool setup(MoreIconsPopup* popup, IconType type) override;
    void transferPlayerToNode(cocos2d::CCNode* node, SimplePlayer* player);
    void addPieceButton(std::string_view suffix, int page, cocos2d::CCArray* targets);
    void updateWithSelectedFiles();
    void updatePieces();
    void goToPage(int page);
    void updateTargets();
    void onClose(cocos2d::CCObject* sender) override;
public:
    static EditIconPopup* create(MoreIconsPopup* popup, IconType type);

    void close();
};
