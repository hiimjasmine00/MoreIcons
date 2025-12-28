#include "BasePopup.hpp"

class MoreIconsPopup : public BasePopup {
protected:
    cocos2d::ccColor3B m_color1;
    cocos2d::ccColor3B m_color2;
    cocos2d::ccColor3B m_colorGlow;
    bool m_glow;
    bool m_dual;

    bool init() override;

    void createMenu(cocos2d::CCNode* gamemodesNode, IconType type);
public:
    static MoreIconsPopup* create();
};
