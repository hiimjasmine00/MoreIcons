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

    std::filesystem::path filePath = path;
    auto fullpath = [[NSBundle mainBundle]
        pathForResource:[NSString stringWithUTF8String:filePath.filename().c_str()]
        ofType:filePath.has_extension() ? nil : [NSString stringWithUTF8String:".plist"]
        inDirectory:[NSString stringWithUTF8String:filePath.parent_path().c_str()]];

    if (fullpath != nil) {
        std::filesystem::path resolvedPath = [fullpath UTF8String];
        return filePath.has_extension() ? resolvedPath : resolvedPath.parent_path() / resolvedPath.stem();
    }
    else return path;
}
