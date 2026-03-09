#include "BasePopup.hpp"

class MoreIconsPopup : public BasePopup {
protected:
    cocos2d::CCNode* m_gamemodesNode;
    bool m_glow;
    bool m_dual;

    bool init() override;
    void onTrash(cocos2d::CCObject* sender);
    void createMenu(IconType type);
    void onVanilla(cocos2d::CCObject* sender);
    void onCustom(cocos2d::CCObject* sender);
    void onLogs(cocos2d::CCObject* sender);
    void onAdd(cocos2d::CCObject* sender);
    void onFolder(cocos2d::CCObject* sender);
public:
    static MoreIconsPopup* create();
};
