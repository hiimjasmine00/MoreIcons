#include "../BasePopup.hpp"
#include <std23/move_only_function.h>

class IconInfo;

class IconPresetPopup : public BasePopup {
protected:
    std23::move_only_function<void(int, IconInfo*)> m_callback;

    bool init(IconType, std::string_view, std23::move_only_function<void(int, IconInfo*)>);
public:
    static IconPresetPopup* create(IconType, std::string_view, std23::move_only_function<void(int, IconInfo*)>);
};
