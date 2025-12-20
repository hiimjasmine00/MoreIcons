#include <cocos2d.h>
#include <texpack.hpp>

class ImageRenderer {
public:
    static texpack::Image getImage(cocos2d::CCNode* node);
    static geode::Result<> save(
        texpack::Packer& packer, const std::filesystem::path& png, const std::filesystem::path& plist, std::string_view name
    );
};
