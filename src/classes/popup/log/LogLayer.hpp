#include "../BasePopup.hpp"

class LogLayer : public BasePopup {
protected:
    bool init(IconType type);
public:
    static LogLayer* create(IconType type);
};
