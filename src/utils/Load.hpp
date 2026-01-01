#include <Geode/utils/cocos.hpp>

struct ImageResult {
    std::string name;
    std::vector<uint8_t> data;
    geode::Ref<cocos2d::CCTexture2D> texture;
    std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>> frames;
    uint32_t width;
    uint32_t height;
};

namespace Load {
    std::string getFrameName(std::string&& frameName, std::string_view name, IconType type);
    geode::Result<std::vector<uint8_t>> readBinary(const std::filesystem::path& path);
    geode::Result<matjson::Value> readPlist(const std::filesystem::path& path);
    bool doesExist(const std::filesystem::path& path);
    geode::Result<cocos2d::CCTexture2D*> createTexture(const std::filesystem::path& path);
    cocos2d::CCTexture2D* createTexture(const uint8_t* data, uint32_t width, uint32_t height);
    void initTexture(cocos2d::CCTexture2D* texture, const uint8_t* data, uint32_t width, uint32_t height, bool premultiplyAlpha = true);
    geode::Result<ImageResult> createFrames(
        const std::filesystem::path& png, const std::filesystem::path& plist, std::string_view name, IconType type,
        std::string_view target = {}, bool premultiply = true
    );
    geode::Result<std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>>> createFrames(
        const std::filesystem::path& path, cocos2d::CCTexture2D* texture, std::string_view name, IconType type,
        std::string_view target = {}, bool fixNames = true
    );
    cocos2d::CCTexture2D* addFrames(const ImageResult& image, std::vector<std::string>& frameNames);
}
