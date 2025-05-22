#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

class IconInfo;

class LazyIcon : public CCMenuItemSpriteExtra {
protected:
    std::string m_texture;
    std::vector<std::string> m_frames;
    cocos2d::CCSprite* m_loadingSprite;
    IconInfo* m_info;
    IconType m_type;
    int m_id;
    bool m_visited;

    bool init(IconType, int, IconInfo*);
    std::pair<std::string, std::string> getTextureNames();
    std::string getIconName();
    void createIcon(const std::string&, const std::string&, const std::vector<std::string>&);
    void createSimpleIcon();
    void createComplexIcon();
public:
    static LazyIcon* create(IconType, int, IconInfo*);

    void activate() override;
    void visit() override;

    ~LazyIcon();
};
