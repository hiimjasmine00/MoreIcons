#include "../BasePopup.hpp"

class IconViewPopup : public BasePopup {
protected:
    bool init(IconType type, bool custom);
public:
    static IconViewPopup* create(IconType type, bool custom);
};
