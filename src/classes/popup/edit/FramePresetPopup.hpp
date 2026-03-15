#include "../BasePopup.hpp"
#include <Geode/utils/async.hpp>
#include <Geode/utils/StringMap.hpp>

class IconInfo;

class FramePresetPopup : public BasePopup {
protected:
    geode::async::TaskHolder<void> m_loader;
    std::vector<std::string> m_textureNames;
    std::vector<geode::Ref<cocos2d::CCTexture2D>> m_textures;
    geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>> m_frames;
    geode::Function<void(cocos2d::CCSpriteFrame*)> m_callback;
    cocos2d::CCSprite* m_loadingSprite;
    IconType m_type;

    bool init(IconType, geode::Function<void(cocos2d::CCSpriteFrame*)>);
    void setupScroll();
    void onSelect(cocos2d::CCObject*);
public:
    static FramePresetPopup* create(IconType, geode::Function<void(cocos2d::CCSpriteFrame*)>);
};
