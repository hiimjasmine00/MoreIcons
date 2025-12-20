#include <Geode/ui/Popup.hpp>

class MoreIconsPopup : public geode::Popup<> {
protected:
    cocos2d::ccColor3B m_color1;
    cocos2d::ccColor3B m_color2;
    cocos2d::ccColor3B m_colorGlow;
    bool m_glow;
    bool m_dual;

    bool setup() override;

    void createMenu(cocos2d::CCNode* gamemodesNode, IconType type);
public:
    static MoreIconsPopup* create();

    void close();
};
