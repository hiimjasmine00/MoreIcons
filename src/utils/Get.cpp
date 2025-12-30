#include "Get.hpp"
#include <Geode/binding/GameManager.hpp>

using namespace geode::prelude;

CCDirector* Get::director = nullptr;
CCFileUtils* Get::fileUtils = nullptr;
GameManager* Get::gameManager = nullptr;
CCSpriteFrameCache* Get::spriteFrameCache = nullptr;
CCTextureCache* Get::textureCache = nullptr;

CCDirector* Get::Director() {
    if (!director) director = CCDirector::sharedDirector();
    return director;
}

CCFileUtils* Get::FileUtils() {
    if (!fileUtils) fileUtils = CCFileUtils::sharedFileUtils();
    return fileUtils;
}

GameManager* Get::GameManager() {
    if (!gameManager) gameManager = GameManager::sharedState();
    return gameManager;
}

CCSpriteFrameCache* Get::SpriteFrameCache() {
    if (!spriteFrameCache) spriteFrameCache = CCSpriteFrameCache::sharedSpriteFrameCache();
    return spriteFrameCache;
}

CCTextureCache* Get::TextureCache() {
    if (!textureCache) textureCache = CCTextureCache::sharedTextureCache();
    return textureCache;
}
