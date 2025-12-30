#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <std23/move_only_function.h>

class IconInfo;

class LazyIcon : public CCMenuItemSpriteExtra {
protected:
    std::string m_name;
    std::string m_error;
    std::string m_sheet;
    std::string m_texture;
    std::vector<std::string> m_frames;
    std23::move_only_function<void()> m_callback;
    std::string_view m_suffix;
    cocos2d::CCSprite* m_loadingSprite;
    IconInfo* m_info;
    IconType m_type;
    bool m_visited;

    bool init(IconType type, int id, IconInfo* info, std::string_view suffix, std23::move_only_function<void()> callback);
    void createIcon();
public:
    static LazyIcon* create(IconType type, int id, IconInfo* info, std::string_view suffix, std23::move_only_function<void()> callback);

    void activate() override;
    void visit() override;

    ~LazyIcon();
};
