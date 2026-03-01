#include <cocos2d.h>
#include <Geode/Enums.hpp>

class IconInfo;

class LazyIcon : public cocos2d::CCMenuItem {
protected:
    cocos2d::CCNode* m_mainNode;
    geode::Function<void()> m_callback;
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

    bool init(IconType type, int id, IconInfo* info, std::string_view suffix, geode::Function<void()> callback);
    void createIcon();
    void setMainNode(CCNode* node);
public:
    static LazyIcon* create(IconType type, int id, IconInfo* info, std::string_view suffix, geode::Function<void()> callback);

    void selected() override;
    void unselected() override;
    void activate() override;
    void visit() override;

    ~LazyIcon() override;
};
