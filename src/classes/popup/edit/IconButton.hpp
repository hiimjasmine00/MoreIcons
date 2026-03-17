#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/utils/async.hpp>

class IconButton : public CCMenuItemSpriteExtra {
protected:
    geode::async::TaskHolder<geode::Result<std::optional<std::filesystem::path>>> m_listener;
    cocos2d::CCSprite* m_unselectedSprite;
    cocos2d::CCLabelBMFont* m_plusLabel;

    bool init() override;
    void activate() override;
public:
    static IconButton* create();

    std::filesystem::path saveIcon(const std::filesystem::path& path);
};
