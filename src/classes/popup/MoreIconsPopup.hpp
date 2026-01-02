#include "BasePopup.hpp"

class MoreIconsPopup : public BasePopup {
protected:
    cocos2d::CCNode* m_gamemodesNode;
    cocos2d::ccColor3B m_color1;
    cocos2d::ccColor3B m_color2;
    cocos2d::ccColor3B m_colorGlow;
    bool m_glow;
    bool m_dual;

    bool init() override;

    void createMenu(IconType type);
public:
    static MoreIconsPopup* create();
};
