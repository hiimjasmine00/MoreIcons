#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <std23/move_only_function.h>

class IconInfo;

class LazyIcon : public CCMenuItemSpriteExtra {
protected:
    std23::move_only_function<void()> m_callback;
    std::string m_name;
    std::string m_error;
    std::string m_key;
    std::filesystem::path m_sheet;
    std::filesystem::path m_texture;
    std::vector<std::string> m_frames;
    std::string m_frameName;
    IconInfo* m_info;
    IconType m_type;
    bool m_visited;

    bool init(IconType type, int id, IconInfo* info, std::string_view suffix, std23::move_only_function<void()> callback);
    void createIcon();
public:
    static LazyIcon* create(IconType type, int id, IconInfo* info, std::string_view suffix, std23::move_only_function<void()> callback);

    void activate() override;
    void setNormalImage(CCNode* image) override;
    void visit() override;

    ~LazyIcon() override;
};
