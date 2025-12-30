#include "IconEditorState.hpp"
#include "../BasePopup.hpp"
#include "../../misc/SimpleIcon.hpp"
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/Task.hpp>

class EditIconPopup : public BasePopup {
protected:
    BasePopup* m_parentPopup;
    geode::EventListener<geode::Task<geode::Result<std::filesystem::path>>> m_listener;
    std::filesystem::path m_selectedPNG;
    std::filesystem::path m_selectedPlist;
    std::vector<std::vector<cocos2d::CCNode*>> m_pages;
    IconEditorState m_state;
    std::unordered_map<std::string, cocos2d::CCSprite*> m_pieces;
    std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>> m_frames;
    SimpleIcon* m_player;
    std::array<Slider*, 6> m_sliders;
    std::array<geode::TextInput*, 6> m_inputs;
    std::string m_suffix;
    const std::vector<cocos2d::CCSprite*>* m_targetsArray;
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
    void createControls(const cocos2d::CCPoint& pos, const char* text, std::string_view id, int offset);
    void updateControls();
    void addPieceButton(const std::string& suffix, int page);
    cocos2d::CCSprite* addColorButton(int& index, cocos2d::CCMenu* menu, const char* text, std::string&& id);
    void updateColors();
    bool updateWithSelectedFiles(bool useSuffix = false);
    cocos2d::CCSpriteFrame* getFrame(const std::string& suffix);
    void updatePieces();
    void goToPage(int page);
    void updateTargets();
    void onClose(cocos2d::CCObject* sender) override;
public:
    static EditIconPopup* create(BasePopup* popup, IconType type);
};
