#include <Geode/ui/Popup.hpp>

class IconInfo;

class MoreInfoPopup : public geode::Popup<IconInfo*> {
protected:
    IconInfo* m_info;
    bool m_toggled;

    bool setup(IconInfo*) override;
    void moveIcon(const std::filesystem::path&, bool);
public:
    static MoreInfoPopup* create(IconInfo*);

    void close();
};
