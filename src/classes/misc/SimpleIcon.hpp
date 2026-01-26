#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <Geode/utils/StringMap.hpp>
#include <span>

struct SpriteDefinition {
    cocos2d::CCPoint position;
    cocos2d::CCPoint scale;
    cocos2d::CCPoint flipped;
    float rotation;
    int zValue;
    int tag;
};

class SimpleIcon : public cocos2d::CCNode {
protected:
    std::vector<CCSpritePlus*> m_spriteParts;
    std::vector<std::vector<SpriteDefinition>> m_definitions;
    geode::utils::StringMap<std::vector<cocos2d::CCSprite*>> m_targets;
    std::vector<std::pair<cocos2d::CCSprite*, float>> m_mainColorSprites;
    std::vector<std::pair<cocos2d::CCSprite*, float>> m_secondaryColorSprites;
    std::vector<cocos2d::CCSprite*> m_glowColorSprites;
    float m_divisor;
    float m_elapsed;
    bool m_looped;

    bool init(IconType type, std::string_view name);
    void createSimpleIcon(IconType type, std::string_view name);
    void createComplexIcon(IconType type, std::string_view name);
    void updateComplexSprite(std::span<SpriteDefinition const> definitions);
public:
    static SimpleIcon* create(IconType type, std::string_view name);

    std::span<cocos2d::CCSprite*> getTargets(std::string_view suffix);
    void setMainColor(const cocos2d::ccColor3B& color);
    void setSecondaryColor(const cocos2d::ccColor3B& color);
    void setGlowColor(const cocos2d::ccColor3B& color);
    void setGlow(bool glow);

    void update(float dt) override;
};
