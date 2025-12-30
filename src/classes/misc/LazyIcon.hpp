#include <Geode/utils/cocos.hpp>
#include <std23/move_only_function.h>

struct SpriteDefinition {
    cocos2d::CCPoint position;
    cocos2d::CCPoint scale;
    cocos2d::CCPoint flipped;
    float rotation;
    int zValue;
    int tag;
};

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
    std::vector<geode::Ref<CCSpritePlus>> m_spriteParts;
    std::vector<std::vector<SpriteDefinition>> m_definitions;
    IconInfo* m_info;
    IconType m_type;
    int m_id;
    float m_divisor;
    float m_elapsed;
    bool m_looped;
    bool m_visited;

    bool init(IconType type, int id, IconInfo* info, std::string_view suffix, std23::move_only_function<void()> callback);
    void createIcon();
    void createSimpleIcon();
    void createComplexIcon();
    void updateComplexSprite(const std::vector<SpriteDefinition>& definitions);
public:
    static LazyIcon* create(IconType type, int id, IconInfo* info, std::string_view suffix, std23::move_only_function<void()> callback);

    void activate() override;
    void visit() override;
    void update(float dt) override;

    ~LazyIcon();
};
