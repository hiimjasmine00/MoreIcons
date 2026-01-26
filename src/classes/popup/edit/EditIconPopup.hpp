#include "IconEditorState.hpp"
#include "../BasePopup.hpp"
#include "../../misc/SimpleIcon.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/Task.hpp>
#include <span>

class EditIconPopup : public BasePopup, public FLAlertLayerProtocol, public TextInputDelegate {
protected:
    BasePopup* m_parentPopup;
    geode::EventListener<geode::Task<geode::Result<std::filesystem::path>>> m_listener;
    std::filesystem::path m_selectedPNG;
    std::filesystem::path m_selectedPlist;
    std::vector<std::vector<cocos2d::CCNode*>> m_pages;
    std::vector<std::string_view> m_required;
    geode::utils::StringMap<cocos2d::CCSprite*> m_pieces;
    geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>> m_frames;
    SimpleIcon* m_player;
    std::array<Slider*, 6> m_sliders;
    std::array<geode::TextInput*, 6> m_inputs;
    std::string m_suffix;
    std::span<cocos2d::CCSprite*> m_targets;
    cocos2d::CCSprite* m_selectSprite;
    cocos2d::CCMenu* m_pieceMenu;
    ButtonSprite* m_pngSprite;
    ButtonSprite* m_plistSprite;
    cocos2d::CCMenu* m_colorMenu;
    cocos2d::CCSprite* m_mainColorSprite;
    cocos2d::CCSprite* m_secondaryColorSprite;
    cocos2d::CCSprite* m_glowColorSprite;
    IconEditorState m_state;
    FrameDefinition* m_definition;
    IconType m_iconType;
    int m_page = 0;
    int m_selectedPage = 0;
    bool m_hasChanged = false;

    bool init(BasePopup* popup, IconType type);
    void onPrevPage(cocos2d::CCObject* sender);
    void onNextPage(cocos2d::CCObject* sender);
    void onLoadState(cocos2d::CCObject* sender);
    void onSaveState(cocos2d::CCObject* sender);
    void onPieceImport(cocos2d::CCObject* sender);
    void onPiecePreset(cocos2d::CCObject* sender);
    void onPieceClear(cocos2d::CCObject* sender);
    void onPNG(cocos2d::CCObject* sender);
    void onPlist(cocos2d::CCObject* sender);
    void onPreset(cocos2d::CCObject* sender);
    void onSave(cocos2d::CCObject* sender);
    void createControls(const cocos2d::CCPoint& pos, const char* text, std::string_view id, int offset);
    void sliderChanged(cocos2d::CCObject* sender);
    void textChanged(CCTextInputNode* input) override;
    void onReset(cocos2d::CCObject* sender);
    void updateControl(int offset, float value, bool slider, bool input, bool definition);
    void updateControls();
    CCMenuItemSpriteExtra* addPieceButton(std::string_view suffix, int page, bool required = true);
    void onSelectPiece(cocos2d::CCObject* sender);
    cocos2d::CCSprite* addColorButton(int type, const char* text, std::string&& id);
    void onColor(cocos2d::CCObject* sender);
    void updateColor(int type, int index);
    bool updateWithSelectedFiles(bool useSuffix = false);
    void updatePieces();
    void goToPage(int page);
    void updateTargets();
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static EditIconPopup* create(BasePopup* popup, IconType type);
};
