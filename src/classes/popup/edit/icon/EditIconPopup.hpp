#include "../FrameDefinition.hpp"
#include "../../BasePopup.hpp"
#include "../../../misc/SimpleIcon.hpp"
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/utils/async.hpp>

class MultiControl;

class EditIconPopup : public BasePopup, public FLAlertLayerProtocol {
protected:
    geode::async::TaskHolder<geode::Result<std::optional<std::filesystem::path>>> m_listener;
    std::filesystem::path m_selectedPNG;
    std::filesystem::path m_selectedPlist;
    std::filesystem::path m_pendingPath;
    std::array<std::filesystem::path, 3> m_pngs;
    std::array<std::filesystem::path, 3> m_plists;
    std::vector<std::vector<cocos2d::CCNode*>> m_pages;
    std::vector<std::string_view> m_required;
    geode::utils::StringMap<cocos2d::CCSprite*> m_pieces;
    geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>> m_frames;
    geode::utils::StringMap<FrameDefinition> m_definitions;
    SimpleIcon* m_player;
    std::array<MultiControl*, 6> m_controls;
    std::string_view m_suffix;
    std::span<cocos2d::CCSprite*> m_targets;
    cocos2d::CCSprite* m_selectSprite;
    cocos2d::CCMenu* m_pieceMenu;
    ButtonSprite* m_pngSprite;
    ButtonSprite* m_plistSprite;
    cocos2d::CCMenu* m_colorMenu;
    cocos2d::CCSprite* m_mainColorSprite;
    cocos2d::CCSprite* m_secondaryColorSprite;
    cocos2d::CCSprite* m_glowColorSprite;
    FrameDefinition* m_definition;
    IconType m_iconType;
    int m_page = 0;
    int m_selectedPage = 0;
    int m_mainColor = 12;
    int m_secondaryColor = 12;
    int m_glowColor = 12;
    bool m_hasChanged = false;

    bool init(IconType type);
    void onPrevPage(cocos2d::CCObject* sender);
    void onNextPage(cocos2d::CCObject* sender);
    void onLoadState(cocos2d::CCObject* sender);
    void onSaveState(cocos2d::CCObject* sender);
    void addFrame(geode::Ref<cocos2d::CCSpriteFrame>&& frame);
    void eraseFrame();
    void onPieceImport(cocos2d::CCObject* sender);
    void onPiecePreset(cocos2d::CCObject* sender);
    void onPieceClear(cocos2d::CCObject* sender);
    void onPNG(cocos2d::CCObject* sender);
    void onPlist(cocos2d::CCObject* sender);
    void onPreset(cocos2d::CCObject* sender);
    void onSave(cocos2d::CCObject* sender);
    void createControls(const cocos2d::CCPoint& pos, const char* text, std::string&& id, int offset);
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
    static EditIconPopup* create(IconType type);
};
