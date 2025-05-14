#include <Geode/ui/Popup.hpp>

class LogLayer : public geode::Popup<IconType> {
protected:
    bool setup(IconType) override;
public:
    static LogLayer* create(IconType);
};
