#include <cocos2d.h>
#include <texpack.hpp>

namespace ImageRenderer {
    texpack::Image getImage(cocos2d::CCNode* node);
    geode::Result<std::vector<uint8_t>> getImage(cocos2d::CCTexture2D* texture);
    texpack::Image getImage(cocos2d::CCSpriteFrame* frame);
    geode::Result<> save(texpack::Packer& packer, const std::filesystem::path& png, const std::filesystem::path& plist, std::string_view name);
}
