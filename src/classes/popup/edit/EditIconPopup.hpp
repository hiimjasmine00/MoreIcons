#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/ui/Popup.hpp>

class MoreIconsPopup;

class EditIconPopup : public geode::Popup<MoreIconsPopup*, IconType> {
protected:
    MoreIconsPopup* m_parentPopup;
    geode::Ref<cocos2d::CCArray> m_pieceArrays;
    geode::Ref<cocos2d::CCDictionary> m_pieceDefinitions;
    geode::Ref<SimplePlayer> m_player;
    Slider* m_offsetXSlider;
    Slider* m_offsetYSlider;
    Slider* m_rotationXSlider;
    Slider* m_rotationYSlider;
    Slider* m_scaleXSlider;
    Slider* m_scaleYSlider;
    cocos2d::CCLabelBMFont* m_offsetXLabel;
    cocos2d::CCLabelBMFont* m_offsetYLabel;
    cocos2d::CCLabelBMFont* m_rotationXLabel;
    cocos2d::CCLabelBMFont* m_rotationYLabel;
    cocos2d::CCLabelBMFont* m_scaleXLabel;
    cocos2d::CCLabelBMFont* m_scaleYLabel;
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
    void goToPage(int page);
    void updateTargets();
    void onClose(cocos2d::CCObject* sender) override;
public:
    static EditIconPopup* create(MoreIconsPopup* popup, IconType type);
};
