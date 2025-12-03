#include "../../../api/MoreIconsAPI.hpp"
#include <texpack.hpp>

class ImageRenderer {
public:
    static texpack::Image getImage(cocos2d::CCNode* node);
    static geode::Result<Autorelease<cocos2d::CCTexture2D>> getTexture(const std::filesystem::path& path);
};
