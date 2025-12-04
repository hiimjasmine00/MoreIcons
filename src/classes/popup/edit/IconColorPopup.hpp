#include <Geode/ui/Popup.hpp>
#include <std23/move_only_function.h>

class IconColorPopup : public geode::Popup<int, std23::move_only_function<void(int)>> {
protected:
    std23::move_only_function<void(int)> m_callback;
    int m_selected;
    int m_original;

    bool setup(int selected, std23::move_only_function<void(int)> callback) override;
    void onClose(cocos2d::CCObject* sender) override;
public:
    static IconColorPopup* create(int selected, std23::move_only_function<void(int)> callback);
};
