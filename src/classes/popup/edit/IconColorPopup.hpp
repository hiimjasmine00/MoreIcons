#include "../BasePopup.hpp"
#include <std23/move_only_function.h>

class IconColorPopup : public BasePopup {
protected:
    std23::move_only_function<void(int)> m_callback;
    int m_selected;
    int m_original;

    bool init(int selected, std23::move_only_function<void(int)> callback);
    void onClose(cocos2d::CCObject* sender) override;
public:
    static IconColorPopup* create(int selected, std23::move_only_function<void(int)> callback);
};
