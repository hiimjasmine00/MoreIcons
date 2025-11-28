#include <Geode/ui/Popup.hpp>

class MoreIconsPopup;

class EditIconPopup : public geode::Popup<MoreIconsPopup*, IconType> {
protected:
    IconType m_iconType;
    float m_offsetX = 0.0f;
    float m_offsetY = 0.0f;
    float m_rotationX = 0.0f;
    float m_rotationY = 0.0f;
    float m_scaleX = 1.0f;
    float m_scaleY = 1.0f;
    bool m_hasChanged = false;

    bool setup(MoreIconsPopup*, IconType) override;

    void onClose(cocos2d::CCObject*) override;
public:
    static EditIconPopup* create(MoreIconsPopup*, IconType);
};
