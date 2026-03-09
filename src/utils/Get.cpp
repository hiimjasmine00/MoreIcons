#include "Get.hpp"
#include <Geode/binding/GameManager.hpp>

using namespace geode::prelude;

CCDirector* Get::director = CCDirector::sharedDirector();
CCFileUtils* Get::fileUtils = CCFileUtils::sharedFileUtils();
GameManager* Get::gameManager = GameManager::sharedState();
CCSpriteFrameCache* Get::spriteFrameCache = CCSpriteFrameCache::sharedSpriteFrameCache();
CCTextureCache* Get::textureCache = CCTextureCache::sharedTextureCache();
