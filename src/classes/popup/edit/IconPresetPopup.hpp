#include "../BasePopup.hpp"

class IconInfo;

class IconPresetPopup : public BasePopup {
protected:
    geode::Function<void(int, IconInfo*)> m_callback;

    bool init(IconType, std::string_view, geode::Function<void(int, IconInfo*)>);
public:
    static IconPresetPopup* create(IconType, std::string_view, geode::Function<void(int, IconInfo*)>);
};
