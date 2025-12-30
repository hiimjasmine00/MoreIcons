#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>

struct SpriteDefinition {
    cocos2d::CCPoint position;
    cocos2d::CCPoint scale;
    cocos2d::CCPoint flipped;
    float rotation;
    int zValue;
    int tag;
};

class SimpleIcon : public cocos2d::CCLayer {
protected:
    std::vector<CCSpritePlus*> m_spriteParts;
    std::vector<std::vector<SpriteDefinition>> m_definitions;
    std::unordered_map<std::string, std::vector<cocos2d::CCSprite*>> m_targets;
    std::vector<std::pair<cocos2d::CCSprite*, float>> m_mainColorSprites;
    std::vector<std::pair<cocos2d::CCSprite*, float>> m_secondaryColorSprites;
    std::vector<cocos2d::CCSprite*> m_glowColorSprites;
    float m_divisor;
    float m_elapsed;
    bool m_looped;

    bool init(IconType type, std::string_view name);
    void createSimpleIcon(IconType type, std::string_view name);
    void createComplexIcon(IconType type, std::string_view name);
    void updateComplexSprite(const std::vector<SpriteDefinition>& definitions);
public:
    static SimpleIcon* create(IconType type, std::string_view name);

    const std::vector<cocos2d::CCSprite*>& getTargets(const std::string& suffix);
    void setColors(const cocos2d::ccColor3B& primary, const cocos2d::ccColor3B& secondary, const cocos2d::ccColor3B& glow);

    void update(float dt) override;
};
