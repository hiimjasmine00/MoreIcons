#include "MoreIcons.hpp"
#include <cocos2d.h>
#include <Foundation/Foundation.h>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

std::string MoreIcons::vanillaTexturePath(const std::string& path, bool skipSuffix) {
    if (CCDirector::get()->getContentScaleFactor() >= 4.0f && !skipSuffix) {
        if (auto highGraphicsAndroid = Loader::get()->getLoadedMod("weebify.high-graphics-android")) {
            auto configDir = highGraphicsAndroid->getConfigDir(false) / Loader::get()->getGameVersion();
            if (std::filesystem::exists(configDir)) return configDir / path;
        }
        return path;
    }

    auto pathString = [NSString stringWithUTF8String:path.c_str()];
    auto fullpath = [[NSBundle mainBundle] pathForResource:pathString ofType:nil inDirectory:pathString];

    return fullpath ? [fullpath UTF8String] : path;
}
