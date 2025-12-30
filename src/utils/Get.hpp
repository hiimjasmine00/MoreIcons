#include <cocos2d.h>
#include <Geode/GeneratedPredeclare.hpp>

namespace Get {
    extern cocos2d::CCDirector* director;
    extern cocos2d::CCFileUtils* fileUtils;
    extern GameManager* gameManager;
    extern cocos2d::CCSpriteFrameCache* spriteFrameCache;
    extern cocos2d::CCTextureCache* textureCache;

    cocos2d::CCDirector* Director();
    cocos2d::CCFileUtils* FileUtils();
    GameManager* GameManager();
    cocos2d::CCSpriteFrameCache* SpriteFrameCache();
    cocos2d::CCTextureCache* TextureCache();
}
