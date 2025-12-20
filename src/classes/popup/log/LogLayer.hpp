#include <Geode/ui/Popup.hpp>

class LogLayer : public geode::Popup<IconType> {
protected:
    bool setup(IconType type) override;
public:
    static LogLayer* create(IconType type);
};
