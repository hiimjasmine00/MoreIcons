#include "../BasePopup.hpp"

class IconInfo;

class FramePresetPopup : public BasePopup {
protected:
    geode::Function<void(int, IconInfo*, int)> m_callback;

    bool init(IconType, geode::Function<void(int, IconInfo*, int)>);
public:
    static FramePresetPopup* create(IconType, geode::Function<void(int, IconInfo*, int)>);
};
