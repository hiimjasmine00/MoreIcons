#include <cocos2d.h>
#include <Geode/GeneratedPredeclare.hpp>

namespace Get {
    extern CCAnimateFrameCache* animateFrameCache;
    extern cocos2d::CCAnimationCache* animationCache;
    extern cocos2d::CCDirector* director;
    extern cocos2d::CCFileUtils* fileUtils;
    extern GameManager* gameManager;
    extern ObjectManager* objectManager;
    extern cocos2d::CCSpriteFrameCache* spriteFrameCache;
    extern cocos2d::CCTextureCache* textureCache;

    CCAnimateFrameCache* AnimateFrameCache();
    cocos2d::CCAnimationCache* AnimationCache();
    cocos2d::CCDirector* Director();
    cocos2d::CCFileUtils* FileUtils();
    GameManager* GameManager();
    ObjectManager* ObjectManager();
    cocos2d::CCSpriteFrameCache* SpriteFrameCache();
    cocos2d::CCTextureCache* TextureCache();
}
