#include <Geode/utils/cocos.hpp>
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
    std::string m_suffix;
    cocos2d::CCSprite* m_loadingSprite;
    geode::Ref<cocos2d::CCArray> m_spriteParts;
    cocos2d::CCAnimation* m_animation;
    IconInfo* m_info;
    IconType m_type;
    int m_id;
    float m_elapsed;
    bool m_looped;
    bool m_visited;

    bool init(IconType, int, IconInfo*, std::string_view, std23::move_only_function<void()>);
    void createIcon();
    void createSimpleIcon();
    void createComplexIcon();
    void updateComplexSprite(cocos2d::CCString*);
public:
    static LazyIcon* create(IconType, int, IconInfo*, std::string_view, std23::move_only_function<void()>);

    void activate() override;
    void visit() override;
    void update(float) override;

    ~LazyIcon();
};
