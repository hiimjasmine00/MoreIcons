#include "../scroll/BiggerScrollLayer.hpp"
#include <Geode/ui/Popup.hpp>

class IconViewPopup : public geode::Popup<IconType, bool> {
protected:
    std::vector<int> m_vanillaIcons;
    std::vector<std::string> m_customIcons;
    std::vector<std::string> m_textures;
    std::vector<std::string> m_frames;
    BiggerScrollLayer* m_scrollLayer;
    IconType m_iconType;
    bool m_custom;

    bool setup(IconType, bool) override;
    void loadCustomIcons();
    void loadVanillaIcons();
    void finishLoadIcons();
    void setupIcons();
public:
    static IconViewPopup* create(IconType, bool);

    ~IconViewPopup();
};
