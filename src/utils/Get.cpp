#include "Get.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/cocos/CCDirector.h>
#include <Geode/cocos/platform/CCFileUtils.h>
#include <Geode/cocos/sprite_nodes/CCSpriteFrameCache.h>
#include <Geode/cocos/textures/CCTextureCache.h>

using namespace geode::prelude;

CCDirector* Get::director = CCDirector::sharedDirector();
CCFileUtils* Get::fileUtils = CCFileUtils::sharedFileUtils();
GameManager* Get::gameManager = GameManager::sharedState();
CCSpriteFrameCache* Get::spriteFrameCache = CCSpriteFrameCache::sharedSpriteFrameCache();
CCTextureCache* Get::textureCache = CCTextureCache::sharedTextureCache();
