#include "Get.hpp"
#include <Geode/binding/CCAnimateFrameCache.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/ObjectManager.hpp>

using namespace geode::prelude;

CCAnimateFrameCache* Get::animateFrameCache = nullptr;
CCAnimationCache* Get::animationCache = nullptr;
CCDirector* Get::director = nullptr;
CCFileUtils* Get::fileUtils = nullptr;
GameManager* Get::gameManager = nullptr;
ObjectManager* Get::objectManager = nullptr;
CCShaderCache* Get::shaderCache = nullptr;
CCSpriteFrameCache* Get::spriteFrameCache = nullptr;
CCTextureCache* Get::textureCache = nullptr;

CCAnimateFrameCache* Get::AnimateFrameCache() {
    if (!animateFrameCache) animateFrameCache = CCAnimateFrameCache::sharedSpriteFrameCache();
    return animateFrameCache;
}

CCAnimationCache* Get::AnimationCache() {
    if (!animationCache) animationCache = CCAnimationCache::sharedAnimationCache();
    return animationCache;
}

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

ObjectManager* Get::ObjectManager() {
    if (!objectManager) objectManager = ObjectManager::instance();
    return objectManager;
}

CCShaderCache* Get::ShaderCache() {
    if (!shaderCache) shaderCache = CCShaderCache::sharedShaderCache();
    return shaderCache;
}

CCSpriteFrameCache* Get::SpriteFrameCache() {
    if (!spriteFrameCache) spriteFrameCache = CCSpriteFrameCache::sharedSpriteFrameCache();
    return spriteFrameCache;
}

CCTextureCache* Get::TextureCache() {
    if (!textureCache) textureCache = CCTextureCache::sharedTextureCache();
    return textureCache;
}
