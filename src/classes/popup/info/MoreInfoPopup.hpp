#include <Geode/ui/Popup.hpp>

class IconInfo;

class MoreInfoPopup : public geode::Popup<IconInfo*> {
protected:
    IconInfo* m_info;
    bool m_toggled;

    bool setup(IconInfo* info) override;
    void moveIcon(const std::filesystem::path& directory, bool trash);
public:
    static MoreInfoPopup* create(IconInfo* info);

    void close();
};
