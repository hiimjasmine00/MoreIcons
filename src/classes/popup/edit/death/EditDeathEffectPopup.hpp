#include "../FrameDefinition.hpp"
#include "../../BasePopup.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/async.hpp>

class EditDeathEffectPopup : public BasePopup, public FLAlertLayerProtocol, public TextInputDelegate {
protected:
    BasePopup* m_parentPopup;
    geode::async::TaskHolder<geode::Result<std::optional<std::filesystem::path>>> m_listener;
    std::filesystem::path m_selectedPNG;
    std::filesystem::path m_selectedPlist;
    std::vector<cocos2d::CCSprite*> m_pieces;
    std::vector<CCMenuItemSpriteExtra*> m_pieceButtons;
    std::vector<geode::Ref<cocos2d::CCSpriteFrame>> m_frames;
    std::array<Slider*, 6> m_sliders;
    std::array<geode::TextInput*, 6> m_inputs;
    cocos2d::CCSprite* m_previewSprite;
    cocos2d::CCNode* m_previewNode;
    cocos2d::CCSprite* m_selectSprite;
    cocos2d::CCMenu* m_pieceMenu;
    ButtonSprite* m_pngSprite;
    ButtonSprite* m_plistSprite;
    std::vector<FrameDefinition> m_definitions;
    FrameDefinition* m_definition;
    int m_page = 0;
    int m_selectedPiece = 0;
    bool m_hasChanged = false;

    bool init(BasePopup* popup);
    void onPrevPage(cocos2d::CCObject* sender);
    void onNextPage(cocos2d::CCObject* sender);
    void onPieceAdd(cocos2d::CCObject* sender);
    void onPieceImport(cocos2d::CCObject* sender);
    void onPiecePreset(cocos2d::CCObject* sender);
    void onPieceRemove(cocos2d::CCObject* sender);
    void onLoadState(cocos2d::CCObject* sender);
    void onSaveState(cocos2d::CCObject* sender);
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
    CCMenuItemSpriteExtra* addPieceButton(int index, cocos2d::CCSpriteFrame* frame);
    void onSelectPiece(cocos2d::CCObject* sender);
    bool updateWithSelectedFiles(bool update = true);
    void updatePieces();
    void updateState();
    void updateTargets();
    void onClose(cocos2d::CCObject* sender) override;
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
public:
    static EditDeathEffectPopup* create(BasePopup* popup);
};
