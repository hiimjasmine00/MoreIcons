#include "../BasePopup.hpp"

class IconInfo;

class ViewIconPopup : public BasePopup {
protected:
    bool init(IconType type, int id, IconInfo* info);
public:
    static ViewIconPopup* create(IconType type, int id, IconInfo* info);
};
