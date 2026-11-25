#include <asp/fs/Path.hpp>
#include <Geode/GeneratedPredeclare.hpp>
#include <texpack.hpp>

namespace ImageRenderer {
    texpack::Image getImage(cocos2d::CCNode* node);
    geode::Result<std::vector<uint8_t>> getImage(cocos2d::CCTexture2D* texture);
    texpack::Image getImage(cocos2d::CCSpriteFrame* frame);
    geode::Result<> save(texpack::Packer& packer, asp::fs::PathView png, asp::fs::PathView plist, std::string_view name);
}
