#include "../BasePopup.hpp"

class IconInfo;

class MoreInfoPopup : public BasePopup {
protected:
    IconInfo* m_info;
    bool m_toggled;

    bool init(IconInfo* info);
    void moveIcon(const std::filesystem::path& directory, bool trash);
public:
    static MoreInfoPopup* create(IconInfo* info);
};
