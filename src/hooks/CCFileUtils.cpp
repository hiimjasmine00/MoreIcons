#include "../utils/Filesystem.hpp"
#include "../utils/Get.hpp"
#include "../utils/Icons.hpp"
#include <Geode/modify/CCFileUtils.hpp>
#include <jasmine/hook.hpp>

using namespace geode::prelude;

class $modify(MIFileUtils, CCFileUtils) {
    static void onModify(ModifyBase<ModifyDerive<MIFileUtils, CCFileUtils>>& self) {
        if (auto hook = jasmine::hook::get(self.m_hooks, "cocos2d::CCFileUtils::fullPathForFilename", Priority::Replace)) {
            hook->setPriority(Priority::Replace);
            Icons::hooks.push_back(hook);
        }
    }

    gd::string fullPathForFilename(const char* filename, bool skipSuffix) {
        std::string_view name = filename;
        if (name.starts_with("icons/") || name.starts_with("PlayerExplosion_") || name.starts_with("streak_") || name.starts_with("shipfire")) {
            gd::string ret = string::pathToString(Icons::vanillaTexturePath(Filesystem::strWide(name), skipSuffix));
            if (!skipSuffix) {
                auto factor = Get::Director()->getContentScaleFactor();
                if (factor >= 4.0f) {
                    auto uhdSuffix = addSuffix(ret, "-uhd");
                    if (isFileExist(uhdSuffix)) return uhdSuffix;
                }
                if (factor >= 2.0f) {
                    auto hdSuffix = addSuffix(ret, "-hd");
                    if (isFileExist(hdSuffix)) return hdSuffix;
                }
            }
            if (isFileExist(ret)) return ret;
        }

        return CCFileUtils::fullPathForFilename(filename, skipSuffix);
    }
};
