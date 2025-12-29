#include "IconEditorState.hpp"
#include "../BasePopup.hpp"
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/Task.hpp>

class EditIconPopup : public BasePopup {
protected:
    BasePopup* m_parentPopup;
    geode::EventListener<geode::Task<geode::Result<std::filesystem::path>>> m_listener;
    std::filesystem::path m_selectedPNG;
    std::filesystem::path m_selectedPlist;
    std::vector<std::vector<geode::Ref<cocos2d::CCNode>>> m_pages;
    IconEditorState m_state;
    StringMap<cocos2d::CCSprite*> m_pieces;
    StringMap<geode::Ref<cocos2d::CCSpriteFrame>> m_frames;
    geode::Ref<SimplePlayer> m_player;
    std::array<geode::Ref<Slider>, 6> m_sliders = {};
    std::array<geode::Ref<geode::TextInput>, 6> m_inputs = {};
    std::string_view m_suffix;
    StringMap<std::vector<geode::Ref<cocos2d::CCNode>>> m_targets;
    std::vector<geode::Ref<cocos2d::CCNode>>* m_targetsArray;
    cocos2d::CCSprite* m_selectSprite;
    cocos2d::CCMenu* m_pieceMenu;
    cocos2d::CCSprite* m_mainColorSprite;
    cocos2d::CCSprite* m_secondaryColorSprite;
    cocos2d::CCSprite* m_glowColorSprite;
    IconType m_iconType;
    int m_page = 0;
    FrameDefinition* m_definition;
    bool m_hasChanged = false;

    bool init(BasePopup* popup, IconType type);
    void createControls(
        const cocos2d::CCPoint& pos, const char* text, std::string_view id, int offset, float min, float max, float def, bool decimals
    );
    void updateControls(std::string_view id, int offset, float min, float max, bool decimals);
    void transferPlayerToNode(cocos2d::CCNode* node, SimplePlayer* player);
    void addPieceButton(std::string_view suffix, int page, std::vector<geode::Ref<cocos2d::CCNode>> targets);
    cocos2d::CCSprite* addColorButton(int& index, cocos2d::CCMenu* menu, const char* text, std::string_view id);
    void updateColors();
    bool updateWithSelectedFiles(std::string_view suffix = {});
    cocos2d::CCSpriteFrame* getFrame(std::string_view suffix);
    void updatePieces();
    void goToPage(int page);
    void updateTargets();
    void onClose(cocos2d::CCObject* sender) override;
public:
    static EditIconPopup* create(BasePopup* popup, IconType type);
};
