#include "IconEditorState.hpp"
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
    IconEditorState m_state;
    geode::Ref<cocos2d::CCDictionary> m_pieces;
    geode::Ref<cocos2d::CCDictionary> m_frames;
    geode::Ref<SimplePlayer> m_player;
    geode::Ref<cocos2d::CCDictionary> m_sliders;
    geode::Ref<cocos2d::CCDictionary> m_inputs;
    std::string_view m_suffix;
    geode::Ref<cocos2d::CCDictionary> m_targets;
    cocos2d::CCArray* m_targetsArray;
    cocos2d::CCSprite* m_selectSprite;
    cocos2d::CCMenu* m_pieceMenu;
    cocos2d::CCSprite* m_mainColorSprite;
    cocos2d::CCSprite* m_secondaryColorSprite;
    cocos2d::CCSprite* m_glowColorSprite;
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
    void createControls(
        const cocos2d::CCPoint& pos, const char* text, std::string_view id, float& value, float min, float max, float def, bool decimals
    );
    void updateControls(std::string_view id, float& value, float min, float max, float def, bool decimals);
    void transferPlayerToNode(cocos2d::CCNode* node, SimplePlayer* player);
    void addPieceButton(std::string_view suffix, int page, cocos2d::CCArray* targets);
    cocos2d::CCSprite* addColorButton(int& index, cocos2d::CCMenu* menu, const char* text, std::string_view id);
    void updateColors();
    bool updateWithSelectedFiles(std::string_view suffix = {});
    void updatePieces();
    void goToPage(int page);
    void updateTargets();
    void onClose(cocos2d::CCObject* sender) override;
public:
    static EditIconPopup* create(MoreIconsPopup* popup, IconType type);

    void close();
};
