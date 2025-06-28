#include <Geode/ui/Popup.hpp>

class MoreIconsPopup : public geode::Popup<> {
protected:
    bool setup() override;
public:
    static MoreIconsPopup* create();

    void close();
};
