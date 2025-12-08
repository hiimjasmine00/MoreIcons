#include <cocos2d.h>
#include <Geode/GeneratedPredeclare.hpp>

class Get {
public:
    static CCAnimateFrameCache* animateFrameCache;
    static cocos2d::CCAnimationCache* animationCache;
    static cocos2d::CCDirector* director;
    static cocos2d::CCFileUtils* fileUtils;
    static GameManager* gameManager;
    static ObjectManager* objectManager;
    static cocos2d::CCSpriteFrameCache* spriteFrameCache;
    static cocos2d::CCTextureCache* textureCache;

    static CCAnimateFrameCache* AnimateFrameCache();
    static cocos2d::CCAnimationCache* AnimationCache();
    static cocos2d::CCDirector* Director();
    static cocos2d::CCFileUtils* FileUtils();
    static GameManager* GameManager();
    static ObjectManager* ObjectManager();
    static cocos2d::CCSpriteFrameCache* SpriteFrameCache();
    static cocos2d::CCTextureCache* TextureCache();
};
