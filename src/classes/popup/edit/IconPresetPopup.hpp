#include <Geode/ui/Popup.hpp>
#include <std23/move_only_function.h>

class IconInfo;

class IconPresetPopup : public geode::Popup<IconType, std::string_view, std23::move_only_function<void(int, IconInfo*)>> {
protected:
    std23::move_only_function<void(int, IconInfo*)> m_callback;

    bool setup(IconType, std::string_view, std23::move_only_function<void(int, IconInfo*)>) override;
public:
    static IconPresetPopup* create(IconType, std::string_view, std23::move_only_function<void(int, IconInfo*)>);
};
