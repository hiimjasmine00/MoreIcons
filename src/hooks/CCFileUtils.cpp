#include <Geode/modify/CCFileUtils.hpp>

using namespace geode::prelude;

class $modify(MIFileUtils, CCFileUtils) {
    gd::string fullPathForFilename(const char* filename, bool skipSuffix) { 
        auto ret = CCFileUtils::fullPathForFilename(filename, skipSuffix);
        if (ret.empty()) return ret;
        auto scaleFactor = CCDirector::sharedDirector()->getContentScaleFactor();
        if (scaleFactor >= 4.0f) {
            auto uhdSuffix = addSuffix(ret, "-uhd");
            if (isFileExist(uhdSuffix)) return uhdSuffix;
        }
        if (scaleFactor >= 2.0f) {
            auto hdSuffix = addSuffix(ret, "-hd");
            if (isFileExist(hdSuffix)) return hdSuffix;
        }
        return ret;
    }
};
