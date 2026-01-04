#include "IconEditorState.hpp"
#include "../BasePopup.hpp"
#include "../../misc/SimpleIcon.hpp"
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/Task.hpp>
#include <span>

class EditIconPopup : public BasePopup {
protected:
    BasePopup* m_parentPopup;
    geode::EventListener<geode::Task<geode::Result<std::filesystem::path>>> m_listener;
    std::filesystem::path m_selectedPNG;
    std::filesystem::path m_selectedPlist;
    std::vector<std::vector<cocos2d::CCNode*>> m_pages;
    std::unordered_map<std::string, cocos2d::CCSprite*> m_pieces;
    std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>> m_frames;
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
    void createControls(const cocos2d::CCPoint& pos, const char* text, std::string_view id, int offset);
    void updateControl(int offset, float value, bool slider, bool input, bool definition);
    void updateControls();
    void selectPiece(const std::string& suffix, int page, const cocos2d::CCPoint& position);
    CCMenuItemSpriteExtra* addPieceButton(const std::string& suffix, int page);
    cocos2d::CCSprite* addColorButton(int type, const char* text, std::string&& id);
    void updateColor(int type, int index);
    bool updateWithSelectedFiles(bool useSuffix = false);
    void updatePieces();
    void goToPage(int page);
    void updateTargets();
    void onClose(cocos2d::CCObject* sender) override;
public:
    static EditIconPopup* create(BasePopup* popup, IconType type);
};
